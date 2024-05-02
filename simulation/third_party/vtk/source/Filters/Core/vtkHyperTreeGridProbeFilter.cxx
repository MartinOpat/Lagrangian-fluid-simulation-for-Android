// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
#include "vtkHyperTreeGridProbeFilter.h"

#include "vtkAbstractArray.h"
#include "vtkCellData.h"
#include "vtkCharArray.h"
#include "vtkDataArray.h"
#include "vtkDataArrayRange.h"
#include "vtkDataSet.h"
#include "vtkDoubleArray.h"
#include "vtkExecutive.h"
#include "vtkFieldData.h"
#include "vtkFloatArray.h"
#include "vtkHyperTreeGrid.h"
#include "vtkHyperTreeGridGeometricLocator.h"
#include "vtkHyperTreeGridLocator.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkSMPTools.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStringArray.h"

#include <cmath>
#include <numeric>
#include <vector>
VTK_ABI_NAMESPACE_BEGIN

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkHyperTreeGridProbeFilter);

//------------------------------------------------------------------------------
vtkCxxSetSmartPointerMacro(vtkHyperTreeGridProbeFilter, Locator, vtkHyperTreeGridLocator);

//------------------------------------------------------------------------------
vtkHyperTreeGridProbeFilter::vtkHyperTreeGridProbeFilter()
  : Locator(vtkSmartPointer<vtkHyperTreeGridGeometricLocator>::New())
{
  this->SetNumberOfInputPorts(2);
}

//------------------------------------------------------------------------------
vtkHyperTreeGridProbeFilter::~vtkHyperTreeGridProbeFilter() = default;

//------------------------------------------------------------------------------
void vtkHyperTreeGridProbeFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  if (this->Locator)
  {
    os << indent << "Locator: ";
    this->Locator->PrintSelf(os, indent.GetNextIndent());
  }
  else
  {
    os << indent << "Locator: none\n";
  }
  os << indent << "PassCellArrays: " << (this->PassCellArrays ? "On\n" : "Off\n");
  os << indent << "PassPointArrays: " << (this->PassPointArrays ? "On\n" : "Off\n");
  os << indent << "PassFieldArrays: " << (this->PassFieldArrays ? "On\n" : "Off\n");
  os << indent << "ValidPointMaskArrayName: " << this->GetValidPointMaskArrayName() << std::endl;
  os << indent << "MaskPoints: " << this->MaskPoints << std::endl;
  os << indent << "ValidPoints: " << this->ValidPoints << std::endl;
}

//------------------------------------------------------------------------------
vtkHyperTreeGridLocator* vtkHyperTreeGridProbeFilter::GetLocator()
{
  return this->Locator;
}

//------------------------------------------------------------------------------
int vtkHyperTreeGridProbeFilter::FillInputPortInformation(int port, vtkInformation* info)
{
  if (port == 0)
  {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  }
  if (port == 1)
  {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkHyperTreeGrid");
  }
  return 1;
}

//------------------------------------------------------------------------------
void vtkHyperTreeGridProbeFilter::SetSourceConnection(vtkAlgorithmOutput* algOutput)
{
  this->SetInputConnection(1, algOutput);
}

//------------------------------------------------------------------------------
void vtkHyperTreeGridProbeFilter::SetSourceData(vtkHyperTreeGrid* input)
{
  this->SetInputData(1, input);
}

//------------------------------------------------------------------------------
vtkHyperTreeGrid* vtkHyperTreeGridProbeFilter::GetSource()
{
  if (this->GetNumberOfInputConnections(1) < 1)
  {
    return nullptr;
  }

  return vtkHyperTreeGrid::SafeDownCast(this->GetExecutive()->GetInputData(1, 0));
}

//------------------------------------------------------------------------------
int vtkHyperTreeGridProbeFilter::RequestInformation(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* sourceInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  outInfo->CopyEntry(sourceInfo, vtkStreamingDemandDrivenPipeline::TIME_STEPS());
  outInfo->CopyEntry(sourceInfo, vtkStreamingDemandDrivenPipeline::TIME_RANGE());

  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
    inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()), 6);
  return 1;
}

//------------------------------------------------------------------------------
int vtkHyperTreeGridProbeFilter::RequestUpdateExtent(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* sourceInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // the updating of output transfers directly into input while the source is entirely updated
  // always
  vtkDataObject* output = vtkDataObject::GetData(outInfo);
  if (output && (output->IsA("vtkUnstructuredGrid") || output->IsA("vtkPolyData")))
  {
    inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER(),
      outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()));
    inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES(),
      outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES()));
    inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(),
      outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS()));
  }
  else
  {
    inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),
      outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT()), 6);
  }

  sourceInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),
    sourceInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()), 6);
  return 1;
}

//------------------------------------------------------------------------------
int vtkHyperTreeGridProbeFilter::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  this->UpdateProgress(0.0);

  // get the input and output
  vtkDataSet* input = vtkDataSet::GetData(inputVector[0], 0);
  vtkHyperTreeGrid* source =
    vtkHyperTreeGrid::SafeDownCast(vtkDataObject::GetData(inputVector[1], 0));
  vtkDataSet* output = vtkDataSet::GetData(outputVector, 0);

  if (!input || !source || !output)
  {
    vtkErrorMacro("Could not get either the input, source or output");
    return 0;
  }

  // setup tolerance
  double tolerance = this->Tolerance;
  if (this->ComputeTolerance)
  {
    double bounds[6];
    source->GetBounds(bounds);
    const double diag = std::sqrt((bounds[1] - bounds[0]) * (bounds[1] - bounds[0]) +
      (bounds[3] - bounds[2]) * (bounds[3] - bounds[2]) +
      (bounds[5] - bounds[4]) * (bounds[5] - bounds[4]));
    tolerance = (diag * 1e-6) / std::pow(source->GetBranchFactor(), source->GetNumberOfLevels());
  }
  this->Locator->SetTolerance(tolerance);

  // setup output
  if (!(this->Initialize(input, source, output)))
  {
    vtkErrorMacro("Could not initialize output arrays");
    return 0;
  }

  this->UpdateProgress(0.1);

  vtkNew<vtkIdList> localPointIds;
  localPointIds->Initialize();
  // run probing on each source individually
  if (!(this->DoProbing(input, source, output, localPointIds)))
  {
    vtkErrorMacro("Could not perform serial probing correctly");
    return 0;
  }

  this->UpdateProgress(0.7);

  // gather and sort results
  if (!(this->Reduce(source, output, localPointIds)))
  {
    vtkErrorMacro("Failed to reduce results");
    return 0;
  }

  this->UpdateProgress(1.0);
  return 1;
}

//------------------------------------------------------------------------------
bool vtkHyperTreeGridProbeFilter::Initialize(
  vtkDataSet* input, vtkHyperTreeGrid* source, vtkDataSet* output)
{
  output->Initialize();

  output->CopyStructure(input);

  if (!(this->PassAttributeData(input, output)))
  {
    vtkErrorMacro("Failed to pass attribute data from input to output");
    return false;
  }

  unsigned int numSourceCellArrays = source->GetCellData()->GetNumberOfArrays();
  for (unsigned int iA = 0; iA < numSourceCellArrays; iA++)
  {
    vtkAbstractArray* da = source->GetCellData()->GetAbstractArray(iA);
    if (!da)
    {
      continue;
    }
    if (!(output->GetPointData()->HasArray(da->GetName())))
    {
      auto localInstance = vtk::TakeSmartPointer(da->NewInstance());
      localInstance->SetName(da->GetName());
      localInstance->SetNumberOfComponents(da->GetNumberOfComponents());
      output->GetPointData()->AddArray(localInstance);
    }
  }

  this->Locator->SetHTG(source);

  // if this is repeatedly called by the pipeline for a composite mesh,
  // you need a new array for each block
  // (that is you need to reinitialize the object)
  this->MaskPoints = vtk::TakeSmartPointer(vtkCharArray::New());
  this->MaskPoints->SetNumberOfComponents(1);
  this->MaskPoints->SetNumberOfTuples(input->GetNumberOfPoints());
  this->FillDefaultArray(this->MaskPoints);
  this->MaskPoints->SetName(!this->ValidPointMaskArrayName.empty()
      ? this->ValidPointMaskArrayName.c_str()
      : "vtkValidPointMask");
  output->GetPointData()->AddArray(this->MaskPoints);

  return true;
}

namespace
{
//------------------------------------------------------------------------------
class ProbingWorklet
{
public:
  vtkSmartPointer<vtkHyperTreeGridLocator> Locator;
  vtkSmartPointer<vtkDataSet> Probe;
  vtkSmartPointer<vtkIdList> ThreadGlobPointIds;
  vtkSmartPointer<vtkIdList> ThreadGlobCellIds;
  struct LocalData
  {
    std::vector<vtkIdType> pointIds;
    std::vector<vtkIdType> cellIds;
  };
  vtkSMPThreadLocal<LocalData> ThreadLocal;

  ProbingWorklet(vtkSmartPointer<vtkDataSet> probe,
    vtkSmartPointer<vtkHyperTreeGridLocator> locator, vtkSmartPointer<vtkIdList> pointIds,
    vtkSmartPointer<vtkIdList> cellIds)
    : Locator(locator)
    , Probe(probe)
    , ThreadGlobPointIds(pointIds)
    , ThreadGlobCellIds(cellIds)
  {
  }

  void Initialize()
  {
    this->ThreadLocal.Local().pointIds = std::vector<vtkIdType>();
    this->ThreadLocal.Local().cellIds = std::vector<vtkIdType>();
  }

  void operator()(vtkIdType begin, vtkIdType end)
  {
    for (vtkIdType iP = begin; iP < end; iP++)
    {
      std::array<double, 3> pt{ 0.0, 0.0, 0.0 };
      this->Probe->GetPoint(iP, pt.data());
      vtkIdType id = this->Locator->Search(pt.data());
      if (!(id < 0))
      {
        this->ThreadLocal.Local().pointIds.emplace_back(iP);
        this->ThreadLocal.Local().cellIds.emplace_back(id);
      }
    }
  }

  void Reduce()
  {
    vtkIdType nPointsFound = 0;
    std::for_each(this->ThreadLocal.begin(), this->ThreadLocal.end(),
      [&nPointsFound](LocalData& ld) { nPointsFound += ld.pointIds.size(); });
    this->ThreadGlobPointIds->SetNumberOfIds(nPointsFound);
    this->ThreadGlobCellIds->SetNumberOfIds(nPointsFound);
    nPointsFound = 0;

    auto mergeThreadResults = [this, &nPointsFound](LocalData& loc) {
      std::copy(
        loc.pointIds.begin(), loc.pointIds.end(), this->ThreadGlobPointIds->begin() + nPointsFound);
      std::copy(
        loc.cellIds.begin(), loc.cellIds.end(), this->ThreadGlobCellIds->begin() + nPointsFound);
      nPointsFound += loc.pointIds.size();
      loc.pointIds.clear();
      loc.cellIds.clear();
    };
    std::for_each(this->ThreadLocal.begin(), this->ThreadLocal.end(), mergeThreadResults);
  }
};
}

//------------------------------------------------------------------------------
bool vtkHyperTreeGridProbeFilter::DoProbing(
  vtkDataSet* probe, vtkHyperTreeGrid* source, vtkDataSet* output, vtkIdList* localPointIds)
{
  // locate all present points of probe
  unsigned int nPoints = probe->GetNumberOfPoints();
  vtkNew<vtkIdList> locCellIds;
  locCellIds->Initialize();
  ::ProbingWorklet worker(probe, this->Locator, localPointIds, locCellIds);

  // XXX: force sequential for now because of https://gitlab.kitware.com/vtk/vtk/-/issues/18629
  vtkSMPTools::LocalScope(
    vtkSMPTools::Config{ 1, "Sequential", false }, [&]() { vtkSMPTools::For(0, nPoints, worker); });

  // copy values from source
  if (locCellIds->GetNumberOfIds() > 0)
  {
    unsigned int numSourceCellArrays = source->GetCellData()->GetNumberOfArrays();
    for (unsigned int iA = 0; iA < numSourceCellArrays; iA++)
    {
      vtkAbstractArray* sourceArray = source->GetCellData()->GetAbstractArray(iA);
      if (!(output->GetPointData()->HasArray(sourceArray->GetName())))
      {
        vtkErrorMacro("Array " << sourceArray->GetName() << " missing in output");
        return false;
      }
      vtkAbstractArray* outputArray =
        output->GetPointData()->GetAbstractArray(sourceArray->GetName());
      outputArray->InsertTuplesStartingAt(0, locCellIds, sourceArray);
    }
  }
  return true;
}

//------------------------------------------------------------------------------
bool vtkHyperTreeGridProbeFilter::Reduce(
  vtkHyperTreeGrid* source, vtkDataSet* output, vtkIdList* localPointIds)
{
  vtkSmartPointer<vtkDataSet> remoteOutput = vtk::TakeSmartPointer(output->NewInstance());
  vtkNew<vtkIdList> remotePointIds;
  // deal with master process
  remoteOutput->CopyStructure(output);
  unsigned int numArrays = source->GetCellData()->GetNumberOfArrays();
  for (unsigned int iA = 0; iA < numArrays; iA++)
  {
    vtkAbstractArray* da = output->GetPointData()->GetAbstractArray(
      source->GetCellData()->GetAbstractArray(iA)->GetName());
    auto localInstance = vtk::TakeSmartPointer(da->NewInstance());
    localInstance->DeepCopy(da);
    remoteOutput->GetPointData()->AddArray(localInstance);
    da->SetNumberOfTuples(output->GetNumberOfPoints());
    this->FillDefaultArray(da);
  }
  this->DealWithRemote(localPointIds, remoteOutput, source, output);
  remoteOutput->Initialize();

  return true;
}

//------------------------------------------------------------------------------
void vtkHyperTreeGridProbeFilter::DealWithRemote(vtkIdList* remotePointIds,
  vtkDataSet* remoteOutput, vtkHyperTreeGrid* source, vtkDataSet* totOutput)
{
  if (remotePointIds->GetNumberOfIds() > 0)
  {
    vtkNew<vtkIdList> iotaIds;
    iotaIds->SetNumberOfIds(remotePointIds->GetNumberOfIds());
    std::iota(iotaIds->begin(), iotaIds->end(), 0);
    unsigned int numArrays = source->GetCellData()->GetNumberOfArrays();
    for (unsigned int iA = 0; iA < numArrays; iA++)
    {
      const char* arrName = source->GetCellData()->GetAbstractArray(iA)->GetName();
      vtkAbstractArray* remoteArray = remoteOutput->GetPointData()->GetAbstractArray(arrName);
      vtkAbstractArray* totArray = totOutput->GetPointData()->GetAbstractArray(arrName);
      totArray->InsertTuples(remotePointIds, iotaIds, remoteArray);
    }
    vtkNew<vtkCharArray> ones;
    ones->SetNumberOfComponents(1);
    ones->SetNumberOfTuples(remotePointIds->GetNumberOfIds());
    auto range = vtk::DataArrayValueRange<1>(ones);
    vtkSMPTools::Fill(range.begin(), range.end(), static_cast<char>(1));
    totOutput->GetPointData()
      ->GetArray(this->GetValidPointMaskArrayName().c_str())
      ->InsertTuples(remotePointIds, iotaIds, ones);
  }
}

//------------------------------------------------------------------------------
void vtkHyperTreeGridProbeFilter::FillDefaultArray(vtkAbstractArray* array) const
{
  if (auto* strArray = vtkStringArray::SafeDownCast(array))
  {
    vtkSMPTools::For(0, strArray->GetNumberOfValues(), [strArray](vtkIdType start, vtkIdType end) {
      for (vtkIdType i = start; i < end; ++i)
      {
        strArray->SetValue(i, "");
      }
    });
  }
  else if (auto* doubleArray = vtkArrayDownCast<vtkDoubleArray>(array))
  {
    doubleArray->Fill(vtkMath::Nan());
  }
  else if (auto* floatArray = vtkArrayDownCast<vtkFloatArray>(array))
  {
    floatArray->Fill(vtkMath::Nan());
  }
  else if (auto* dArray = vtkArrayDownCast<vtkDataArray>(array))
  {
    dArray->Fill(0);
  }
  else
  {
    vtkGenericWarningMacro("Array is not a vtkDataArray nor is it a vtkStringArray and will not be "
                           "filled with default values.");
  }
}

//------------------------------------------------------------------------------
// Straight up copy from vtkProbeFilter
bool vtkHyperTreeGridProbeFilter::PassAttributeData(vtkDataSet* input, vtkDataSet* output)
{
  // copy point data arrays
  if (this->PassPointArrays)
  {
    output->GetPointData()->PassData(input->GetPointData());
  }

  // copy cell data arrays
  if (this->PassCellArrays)
  {
    output->GetCellData()->PassData(input->GetCellData());
  }

  if (this->PassFieldArrays)
  {
    // nothing to do, vtkDemandDrivenPipeline takes care of that.
  }
  else
  {
    output->GetFieldData()->Initialize();
  }
  return true;
}

//------------------------------------------------------------------------------
// Straight up copy from vtkProbeFilter
vtkIdTypeArray* vtkHyperTreeGridProbeFilter::GetValidPoints()
{
  if (this->MaskPoints && this->MaskPoints->GetMTime() > this->ValidPoints->GetMTime())
  {
    char* maskArray = this->MaskPoints->GetPointer(0);
    vtkIdType numPts = this->MaskPoints->GetNumberOfTuples();
    vtkIdType numValidPoints = std::count(maskArray, maskArray + numPts, static_cast<char>(1));
    this->ValidPoints->Allocate(numValidPoints);
    for (vtkIdType i = 0; i < numPts; ++i)
    {
      if (maskArray[i])
      {
        this->ValidPoints->InsertNextValue(i);
      }
    }
    this->ValidPoints->Modified();
  }

  return this->ValidPoints;
}
VTK_ABI_NAMESPACE_END
