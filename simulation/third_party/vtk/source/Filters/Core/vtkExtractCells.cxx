// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-FileCopyrightText: Copyright (c) Sandia Corporation
// SPDX-License-Identifier: BSD-3-Clause

#include "vtkExtractCells.h"

#include "vtkArrayDispatch.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointSet.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyDataToUnstructuredGrid.h"
#include "vtkSMPThreadLocalObject.h"
#include "vtkSMPTools.h"
#include "vtkTimeStamp.h"
#include "vtkUnsignedCharArray.h"
#include "vtkUnstructuredGrid.h"

#include <algorithm>
#include <numeric>
#include <vector>

VTK_ABI_NAMESPACE_BEGIN
namespace
{

struct ExtractedCellsT
{
  vtkSmartPointer<vtkCellArray> Connectivity;
  vtkSmartPointer<vtkUnsignedCharArray> CellTypes;
  vtkSmartPointer<vtkIdTypeArray> Faces;
  vtkSmartPointer<vtkIdTypeArray> FaceLocations;
};

//=============================================================================
/*
 * These work types help us reuse the same code for extracting elements
 * when a smaller subset is being extracted or everything is being extracted
 * with ease.
 */
struct AllElementsWork
{
  vtkIdType NumberOfPoints;
  vtkIdType NumberOfCells;

  // PointWork API
  inline vtkIdType GetNumberOfPoints() const { return this->NumberOfPoints; }
  inline vtkIdType GetPointId(vtkIdType index) const { return index; }

  // CellWork API
  inline vtkIdType GetNumberOfCells() const { return this->NumberOfCells; }
  inline vtkIdType GetCellId(vtkIdType index) const { return index; }
};

struct SubsetCellsWork
{
  const vtkIdType* CellListPtr;
  const vtkIdType* PointMapPtr;
  vtkIdType NumberOfCells;

  inline vtkIdType GetNumberOfCells() const { return this->NumberOfCells; }
  inline vtkIdType GetCellId(vtkIdType index) const { return this->CellListPtr[index]; }
  inline vtkIdType GetPointId(vtkIdType id) const { return this->PointMapPtr[id]; }
};

struct SubsetPointsWork
{
  const vtkSmartPointer<vtkIdList>& PointIdsToExtract;
  inline vtkIdType GetNumberOfPoints() const { return this->PointIdsToExtract->GetNumberOfIds(); }
  inline vtkIdType GetPointId(vtkIdType index) const
  {
    return this->PointIdsToExtract->GetId(index);
  }
};

//=============================================================================
template <typename PointWorkT>
struct ExtractPointsWorker
{
  template <typename PointsArrayT>
  void operator()(PointsArrayT* pointsArray, const PointWorkT& pointWork, vtkDataSet* input)
  {
    vtkSMPTools::For(0, pointWork.GetNumberOfPoints(), [&](vtkIdType begin, vtkIdType end) {
      double point[3];
      auto points = vtk::DataArrayTupleRange<3>(pointsArray);
      for (vtkIdType ptId = begin; ptId < end; ++ptId)
      {
        const vtkIdType origPtId = pointWork.GetPointId(ptId);
        input->GetPoint(origPtId, point);
        points[ptId][0] = point[0];
        points[ptId][1] = point[1];
        points[ptId][2] = point[2];
      }
    });
  }
};

//------------------------------------------------------------------------------
/* This function returns a new vtkPoints extracted from the `input`.
 * The points to extract are identified by the `PointWork`:
 *  `PointWork::GetNumberOfPoints`: total number of points to extract
 *  `PointWork::GetPointId(idx)`: original pt id for extracted point at the `idx`
 */
template <typename PointWorkT>
vtkSmartPointer<vtkPoints> ExtractPoints(
  vtkDataSet* input, int outputPointsPrecision, const PointWorkT& work)
{
  vtkNew<vtkPoints> pts;
  // set precision for the points in the output
  if (outputPointsPrecision == vtkAlgorithm::DEFAULT_PRECISION)
  {
    vtkPointSet* inputPointSet = vtkPointSet::SafeDownCast(input);
    if (inputPointSet && inputPointSet->GetPoints())
    {
      pts->SetDataType(inputPointSet->GetPoints()->GetDataType());
    }
    else
    {
      pts->SetDataType(VTK_FLOAT);
    }
  }
  else if (outputPointsPrecision == vtkAlgorithm::SINGLE_PRECISION)
  {
    pts->SetDataType(VTK_FLOAT);
  }
  else if (outputPointsPrecision == vtkAlgorithm::DOUBLE_PRECISION)
  {
    pts->SetDataType(VTK_DOUBLE);
  }
  pts->SetNumberOfPoints(work.GetNumberOfPoints());
  auto array = pts->GetData();

  ExtractPointsWorker<PointWorkT> worker;
  if (!vtkArrayDispatch::Dispatch::Execute(array, worker, work, input))
  {
    worker(array, work, input);
  }
  return pts;
}

//------------------------------------------------------------------------------
/**
 * Adds `vtkOriginalCellIds` array, if not already present in `outCD`.
 * `CellWorkT::GetNumberOfCells`: total number of cells being extracted.
 * `CellWorkT::GetCellId(idx)`: original cell id for extracted cell at index `idx`.
 */
template <typename CellWorkT>
void AddOriginalCellIds(vtkCellData* outCD, const CellWorkT& work)
{
  // add vtkOriginalCellIds array, if needed.
  if (outCD->GetArray("vtkOriginalCellIds") == nullptr)
  {
    const auto numCells = work.GetNumberOfCells();
    vtkNew<vtkIdTypeArray> ids;
    ids->SetName("vtkOriginalCellIds");
    ids->SetNumberOfValues(numCells);
    vtkSMPTools::For(0, numCells, [&ids, &work](vtkIdType start, vtkIdType end) {
      for (vtkIdType cc = start; cc < end; ++cc)
      {
        ids->SetValue(cc, work.GetCellId(cc));
      }
    });
    outCD->AddArray(ids);
  }
}

//-----------------------------------------------------------------------------
// Keep track of output information within each batch of cells - this
// information is eventually rolled up into offsets into the cell
// connectivity and offsets arrays so that separate threads know where to
// write their data. We need to know the connectivity size of the output cells.
struct ExtractCellsBatch
{
  vtkIdType CellsConnectivitySize;
  vtkIdType BeginCellIndex;
  vtkIdType EndCellIndex;
  vtkIdType BeginCellsConnectivity;

  ExtractCellsBatch()
    : CellsConnectivitySize(0)
    , BeginCellIndex(0)
    , EndCellIndex(0)
    , BeginCellsConnectivity(0)
  {
  }
};

//-----------------------------------------------------------------------------
struct ExtractCellsBatchInfo
{
  vtkIdType BatchSize;
  std::vector<ExtractCellsBatch> Batches;
};

//------------------------------------------------------------------------------
/* Extracts cells identified by `work` from the input.
 * Returns ExtractedCellsT with connectivity and cell-types array set.
 */
template <typename CellWorkT>
ExtractedCellsT ExtractCells(vtkDataSet* input, const CellWorkT& work, unsigned int batchSize)
{
  const auto outputNumCells = work.GetNumberOfCells();

  ExtractedCellsT result;
  result.CellTypes.TakeReference(vtkUnsignedCharArray::New());
  result.CellTypes->SetNumberOfValues(outputNumCells);

  // ensure that internal structures are initialized.
  input->GetCell(0);

  // set cell types
  vtkSMPTools::For(0, outputNumCells, [&](vtkIdType begin, vtkIdType end) {
    for (vtkIdType cc = begin; cc < end; ++cc)
    {
      result.CellTypes->SetValue(cc, input->GetCellType(work.GetCellId(cc)));
    }
  });

  // initialize batches
  ExtractCellsBatchInfo batchInfo;
  batchInfo.BatchSize = static_cast<vtkIdType>(batchSize);
  size_t numberOfBatches = static_cast<size_t>(((outputNumCells - 1) / batchSize) + 1);
  batchInfo.Batches.resize(numberOfBatches);

  // figure out the connectivity size and the begin values for each batch
  vtkSMPThreadLocalObject<vtkIdList> TLCellPointIds;
  vtkSMPTools::For(0, static_cast<vtkIdType>(numberOfBatches), [&](vtkIdType begin, vtkIdType end) {
    vtkIdType numCellPts, cellId, cellIndex;
    const vtkIdType* cellPts;
    auto& cellPointIds = TLCellPointIds.Local();
    for (vtkIdType batchId = begin; batchId < end; ++batchId)
    {
      ExtractCellsBatch& batch = batchInfo.Batches[batchId];
      batch.BeginCellIndex = batchId * batchInfo.BatchSize;
      batch.EndCellIndex = (batch.BeginCellIndex + batchInfo.BatchSize > outputNumCells
          ? outputNumCells
          : batch.BeginCellIndex + batchInfo.BatchSize);
      for (cellIndex = batch.BeginCellIndex; cellIndex < batch.EndCellIndex; ++cellIndex)
      {
        cellId = work.GetCellId(cellIndex);
        input->GetCellPoints(cellId, numCellPts, cellPts, cellPointIds);
        batch.CellsConnectivitySize += numCellPts;
      }
    }
  });
  // assign BeginCellsConnectivity and calculate connectivity size
  vtkIdType connectivitySize = 0, beginCellsConnectivity = 0;
  for (auto& batch : batchInfo.Batches)
  {
    batch.BeginCellsConnectivity = beginCellsConnectivity;
    beginCellsConnectivity += batch.CellsConnectivitySize;
    connectivitySize += batch.CellsConnectivitySize;
  }

  // set cell array connectivity
  vtkNew<vtkIdTypeArray> connectivity;
  connectivity->SetNumberOfValues(connectivitySize);
  // set cell array offsets
  vtkNew<vtkIdTypeArray> offsets;
  offsets->SetNumberOfValues(outputNumCells + 1);
  vtkSMPTools::For(0, static_cast<vtkIdType>(numberOfBatches), [&](vtkIdType begin, vtkIdType end) {
    vtkIdType numCellPts, cellId, cellIndex, ptId;
    const vtkIdType* cellPts;
    auto& cellPointIds = TLCellPointIds.Local();
    for (vtkIdType batchId = begin; batchId < end; ++batchId)
    {
      ExtractCellsBatch& batch = batchInfo.Batches[batchId];
      for (cellIndex = batch.BeginCellIndex; cellIndex < batch.EndCellIndex; ++cellIndex)
      {
        cellId = work.GetCellId(cellIndex);
        input->GetCellPoints(cellId, numCellPts, cellPts, cellPointIds);
        offsets->SetValue(cellIndex, batch.BeginCellsConnectivity);
        for (ptId = 0; ptId < numCellPts; ++ptId)
        {
          connectivity->SetValue(batch.BeginCellsConnectivity++, work.GetPointId(cellPts[ptId]));
        }
      }
    }
  });
  // set last offset
  offsets->SetValue(outputNumCells, connectivitySize);
  // set cell array
  result.Connectivity.TakeReference(vtkCellArray::New());
  result.Connectivity->SetData(offsets, connectivity);
  return result;
}

//------------------------------------------------------------------------------
/**
 * Extract polyhedral cell-face information form input. Adds `Faces` and
 * `FaceLocations` to `result`.
 */
template <typename CellWorkT>
void ExtractPolyhedralFaces(
  ExtractedCellsT& result, vtkUnstructuredGrid* input, const CellWorkT& work)
{
  const auto numCells = work.GetNumberOfCells();
  auto inFaceLocations = input->GetFaceLocations();
  auto inFaces = input->GetFaces();

  result.FaceLocations.TakeReference(vtkIdTypeArray::New());
  result.FaceLocations->SetNumberOfTuples(numCells);

  vtkIdType outFacesSize = 0;
  for (vtkIdType cc = 0; cc < numCells; ++cc)
  {
    const auto loc = inFaceLocations->GetValue(work.GetCellId(cc));
    if (loc == -1)
    {
      // not a polyhedral cell
      result.FaceLocations->SetTypedComponent(cc, 0, -1);
    }
    else
    {
      result.FaceLocations->SetTypedComponent(cc, 0, outFacesSize);
      vtkIdType* pfaces_start = inFaces->GetPointer(loc);
      vtkIdType* pfaces = pfaces_start;
      const auto nfaces = (*pfaces++);
      for (vtkIdType face = 0; face < nfaces; ++face)
      {
        const auto npts = (*pfaces++);
        pfaces += npts;
      }
      outFacesSize += static_cast<vtkIdType>(std::distance(pfaces_start, pfaces));
    }
  }

  // Now copy polyhedron Faces.
  result.Faces.TakeReference(vtkIdTypeArray::New());
  result.Faces->SetNumberOfTuples(outFacesSize);

  vtkSMPTools::For(0, numCells, [&](vtkIdType start, vtkIdType end) {
    for (vtkIdType cc = start; cc < end; ++cc)
    {
      const auto inLoc = inFaceLocations->GetValue(work.GetCellId(cc));
      if (inLoc == -1)
      {
        continue;
      }
      const auto outLoc = result.FaceLocations->GetValue(cc);

      auto iptr = inFaces->GetPointer(inLoc);
      auto optr = result.Faces->GetPointer(outLoc);
      const auto nfaces = *iptr++;
      *optr++ = nfaces;
      for (vtkIdType face = 0; face < nfaces; ++face)
      {
        const auto npts = (*iptr++);
        *optr++ = npts;
        std::transform(
          iptr, iptr + npts, optr, [&work](vtkIdType id) { return work.GetPointId(id); });
        optr += npts;
        iptr += npts;
      }
    }
  });
}

//------------------------------------------------------------------------------
vtkSmartPointer<vtkIdList> GeneratePointMap(
  vtkDataSet* input, vtkIdList* cellList, vtkIdType& outNumPoints)
{
  vtkNew<vtkIdList> pointMap;
  pointMap->SetNumberOfIds(input->GetNumberOfPoints());
  pointMap->Fill(0);
  const vtkIdType numberOutputCells = cellList->GetNumberOfIds();

  vtkSMPThreadLocalObject<vtkIdList> TLCellPointIds;
  // ensure that internal structures are initialized.
  input->GetCell(0);

  auto pointMapPtr = pointMap->GetPointer(0);
  vtkSMPTools::For(0, numberOutputCells, [&](vtkIdType begin, vtkIdType end) {
    vtkIdType npts, cellId;
    const vtkIdType* ptids;
    auto& cellPointIds = TLCellPointIds.Local();
    for (vtkIdType cellIndex = begin; cellIndex < end; ++cellIndex)
    {
      cellId = cellList->GetId(cellIndex);
      input->GetCellPoints(cellId, npts, ptids, cellPointIds);
      for (vtkIdType i = 0; i < npts; ++i)
      {
        pointMapPtr[ptids[i]] = 1;
      }
    }
  });
  // convert flags to map where index is old id, value is new id and -1 means
  // the point is to be discarded.
  vtkIdType nextid = 0;
  for (auto& pt : *pointMap)
  {
    pt = pt ? nextid++ : -1;
  }
  outNumPoints = nextid;
  return pointMap;
}

//------------------------------------------------------------------------------
vtkSmartPointer<vtkIdList> ConvertToPointIdsToExtract(vtkIdList* pointMap)
{
  const auto numberOfInputPoints = pointMap->GetNumberOfIds();
  vtkNew<vtkIdList> srcIds;
  srcIds->Allocate(numberOfInputPoints);
  auto pointMapPtr = pointMap->GetPointer(0);
  for (vtkIdType cc = 0; cc < numberOfInputPoints; ++cc)
  {
    if (pointMapPtr[cc] != -1)
    {
      srcIds->InsertNextId(cc);
    }
  }
  srcIds->Squeeze();
  return srcIds;
}

} // end anonymous namespace

//=============================================================================
class vtkExtractCellsIdList : public vtkIdList
{
public:
  vtkTypeMacro(vtkExtractCellsIdList, vtkIdList);
  void PrintSelf(ostream& os, vtkIndent indent) override
  {
    this->Superclass::PrintSelf(os, indent);
    os << indent << "SortTime: " << this->SortTime << endl;
  }
  static vtkExtractCellsIdList* New() { VTK_STANDARD_NEW_BODY(vtkExtractCellsIdList); }

public:
  vtkIdType Prepare(vtkIdType numInputCells, vtkExtractCells* self)
  {
    if (numInputCells == 0 || this->GetNumberOfIds() == 0)
    {
      return 0;
    }
    if (!self->GetAssumeSortedAndUniqueIds() && (self->GetMTime() > this->SortTime))
    {
      vtkSMPTools::Sort(this->begin(), this->end());
      auto last = std::unique(this->begin(), this->end());
      this->SetNumberOfIds(static_cast<vtkIdType>(std::distance(this->begin(), last)));
      this->SortTime.Modified();
    }
    // check if ids larger than number of cells exist or negative.
    if (this->GetId(this->GetNumberOfIds() - 1) >= numInputCells || this->GetId(0) < 0)
    {
      if (this->GetId(this->GetNumberOfIds() - 1) >= numInputCells && this->GetId(0) >= 0)
      {
        auto smallest = this->begin();
        auto largest = std::upper_bound(this->begin(), this->end(), numInputCells - 1);
        this->Resize(static_cast<vtkIdType>(std::distance(smallest, largest)));
      }
      else
      {
        // remove them
        auto smallest = std::lower_bound(this->begin(), this->end(), 0);
        auto largest = this->GetId(this->GetNumberOfIds() - 1) >= numInputCells
          ? std::upper_bound(this->begin(), this->end(), numInputCells - 1)
          : this->end();
        std::copy(smallest, largest, this->begin());
        this->Resize(static_cast<vtkIdType>(std::distance(this->begin(), largest)));
      }
    }
    return this->GetNumberOfIds();
  }

protected:
  vtkExtractCellsIdList() = default;
  ~vtkExtractCellsIdList() override = default;

private:
  vtkExtractCellsIdList(const vtkExtractCellsIdList&) = delete;
  void operator=(const vtkExtractCellsIdList&) = delete;

  vtkTimeStamp SortTime;
};

//=============================================================================
vtkStandardNewMacro(vtkExtractCells);
//------------------------------------------------------------------------------
vtkExtractCells::vtkExtractCells()
{
  this->CellList = vtkSmartPointer<vtkExtractCellsIdList>::New();
}

//------------------------------------------------------------------------------
vtkExtractCells::~vtkExtractCells() = default;

//------------------------------------------------------------------------------
void vtkExtractCells::SetCellList(vtkIdList* l)
{
  this->CellList = vtkSmartPointer<vtkExtractCellsIdList>::New();
  if (l != nullptr)
  {
    this->AddCellList(l);
  }
  this->Modified();
}

//------------------------------------------------------------------------------
void vtkExtractCells::AddCellList(vtkIdList* l)
{
  const vtkIdType inputSize = l ? l->GetNumberOfIds() : 0;
  if (inputSize == 0)
  {
    return;
  }
  this->AddCellIds(l->GetPointer(0), l->GetNumberOfIds());
}

//------------------------------------------------------------------------------
void vtkExtractCells::SetCellIds(const vtkIdType* ptr, vtkIdType numValues)
{
  this->CellList = vtkSmartPointer<vtkExtractCellsIdList>::New();
  if (ptr != nullptr && numValues > 0)
  {
    this->AddCellIds(ptr, numValues);
  }
  this->Modified();
}

//------------------------------------------------------------------------------
void vtkExtractCells::AddCellIds(const vtkIdType* ptr, vtkIdType numValues)
{
  auto& cellIds = this->CellList;
  const vtkIdType oldSize = cellIds->GetNumberOfIds();
  const vtkIdType newSize = oldSize + numValues;
  if (oldSize != 0)
  {
    cellIds->Resize(newSize);
  }
  cellIds->SetNumberOfIds(newSize);
  vtkSMPTools::For(0, numValues, [&](vtkIdType begin, vtkIdType end) {
    std::copy(ptr + begin, ptr + end, cellIds->GetPointer(oldSize + begin));
  });
  this->Modified();
}

//------------------------------------------------------------------------------
void vtkExtractCells::AddCellRange(vtkIdType from, vtkIdType to)
{
  if (to < from || to < 0)
  {
    vtkWarningMacro("Bad cell range: (" << to << "," << from << ")");
    return;
  }

  // This range specification is inconsistent with C++. Left for backward
  // compatibility reasons.  Add 1 to `to` to make it consistent.
  ++to;

  auto& cellIds = this->CellList;
  const vtkIdType oldSize = cellIds->GetNumberOfIds();
  const vtkIdType numValues = to - from;
  const vtkIdType newSize = oldSize + numValues;
  if (oldSize != 0)
  {
    cellIds->Resize(newSize);
  }
  cellIds->SetNumberOfIds(newSize);
  vtkSMPTools::For(0, numValues, [&](vtkIdType begin, vtkIdType end) {
    std::iota(
      cellIds->GetPointer(oldSize + begin), cellIds->GetPointer(oldSize + end), from + begin);
  });
  this->Modified();
}

//------------------------------------------------------------------------------
int vtkExtractCells::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  // get the input and output
  vtkDataSet* input = vtkDataSet::GetData(inputVector[0]);
  vtkUnstructuredGrid* output = vtkUnstructuredGrid::GetData(outputVector);

  auto inPD = input->GetPointData();
  auto inCD = input->GetCellData();
  auto outPD = output->GetPointData();
  auto outCD = output->GetCellData();

  // copy all arrays, including global ids etc.
  outPD->CopyAllOn();
  outCD->CopyAllOn();

  const vtkIdType inputNumCells = input->GetNumberOfCells();
  const vtkIdType outputNumbCells =
    this->ExtractAllCells ? inputNumCells : this->CellList->Prepare(inputNumCells, this);

  // Handle simple cases, first.
  // Check if no cells are to be extracted
  if (outputNumbCells == 0)
  {
    // set up a ugrid with same data arrays as input, but
    // no points, cells or data.
    output->Allocate(1);
    outPD->CopyAllocate(inPD, 1);
    outCD->CopyAllocate(inCD, 1);
    vtkNew<vtkPoints> pts;
    pts->SetNumberOfPoints(0);
    output->SetPoints(pts);
    return 1;
  }
  else if (inputNumCells == outputNumbCells)
  {
    // Check if all cells are to be extracted.
    // `Copy` will ShallowCopy input if input is vtkUnstructuredGrid, else
    // convert it to an unstructured grid.
    return this->Copy(input, output) ? 1 : 0;
  }
  if (this->CheckAbort())
  {
    return 1;
  }

  // Build point map for selected cells.
  vtkIdType outputNumPoints;
  const auto pointMap = ::GeneratePointMap(input, this->CellList, outputNumPoints);
  auto chosenPtIds = ::ConvertToPointIdsToExtract(pointMap);
  this->UpdateProgress(0.25);
  if (this->CheckAbort())
  {
    return 1;
  }

  // Copy cell and point data first, since that's easy enough.
  outCD->CopyAllocate(inCD, outputNumbCells);
  outCD->CopyData(inCD, this->CellList);
  outPD->CopyAllocate(inPD, outputNumPoints);
  outPD->CopyData(inPD, chosenPtIds);

  const SubsetCellsWork work{ this->CellList->GetPointer(0), pointMap->GetPointer(0),
    outputNumbCells };
  if (this->PassThroughCellIds)
  {
    ::AddOriginalCellIds(outCD, work);
  }
  this->UpdateProgress(0.5);
  if (this->CheckAbort())
  {
    return 1;
  }

  // Get new points
  auto pts = ::ExtractPoints(input, this->OutputPointsPrecision, SubsetPointsWork{ chosenPtIds });
  output->SetPoints(pts);
  this->UpdateProgress(0.75);
  if (this->CheckAbort())
  {
    return 1;
  }

  // Extract cells
  auto cells = ::ExtractCells(input, work, this->BatchSize);
  this->UpdateProgress(0.85);
  if (this->CheckAbort())
  {
    return 1;
  }

  // Handle polyhedral cells
  auto inputUG = vtkUnstructuredGrid::SafeDownCast(input);
  if (inputUG && inputUG->GetFaces() && inputUG->GetFaceLocations() &&
    inputUG->GetFaceLocations()->GetRange(0)[1] != -1)
  {
    ::ExtractPolyhedralFaces(cells, inputUG, work);
  }
  output->SetCells(cells.CellTypes, cells.Connectivity, cells.FaceLocations, cells.Faces);
  this->UpdateProgress(1.00);

  return 1;
}

//------------------------------------------------------------------------------
bool vtkExtractCells::Copy(vtkDataSet* input, vtkUnstructuredGrid* output)
{
  if (vtkUnstructuredGrid::SafeDownCast(input))
  {
    output->ShallowCopy(input);
    return true;
  }

  if (vtkPolyData::SafeDownCast(input))
  {
    vtkNew<vtkPolyDataToUnstructuredGrid> converter;
    converter->SetInputData(input);
    converter->SetContainerAlgorithm(this);
    converter->Update();
    output->ShallowCopy(converter->GetOutput());
    return true;
  }

  if (vtkPointSet::SafeDownCast(input))
  {
    // pass points along.
    output->vtkPointSet::ShallowCopy(input);
  }
  else
  {
    // copy points manually.
    const vtkIdType numPoints = input->GetNumberOfPoints();
    auto pts = ::ExtractPoints(input, this->OutputPointsPrecision, AllElementsWork{ numPoints, 0 });
    output->SetPoints(pts);
  }

  const auto numCells = input->GetNumberOfCells();
  auto cells = ::ExtractCells(input, AllElementsWork{ 0, numCells }, this->BatchSize);
  output->SetCells(cells.CellTypes, cells.Connectivity, nullptr, nullptr);

  // copy cell/point arrays.
  output->GetPointData()->ShallowCopy(input->GetPointData());
  output->GetCellData()->ShallowCopy(input->GetCellData());
  if (this->PassThroughCellIds)
  {
    ::AddOriginalCellIds(output->GetCellData(), AllElementsWork{ 0, numCells });
  }
  return true;
}

//------------------------------------------------------------------------------
int vtkExtractCells::FillInputPortInformation(int, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}

//------------------------------------------------------------------------------
void vtkExtractCells::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ExtractAllCells: " << this->ExtractAllCells << endl;
  os << indent << "AssumeSortedAndUniqueIds: " << this->AssumeSortedAndUniqueIds << endl;
}
VTK_ABI_NAMESPACE_END
