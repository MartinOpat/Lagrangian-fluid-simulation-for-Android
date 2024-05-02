// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
#include "vtkmTriangleMeshPointNormals.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"

#include "vtkmlib/ArrayConverters.h"
#include "vtkmlib/PolyDataConverter.h"

#include "vtkm/filter/vector_analysis/SurfaceNormals.h"

VTK_ABI_NAMESPACE_BEGIN
vtkStandardNewMacro(vtkmTriangleMeshPointNormals);

//------------------------------------------------------------------------------
void vtkmTriangleMeshPointNormals::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//------------------------------------------------------------------------------
vtkmTriangleMeshPointNormals::vtkmTriangleMeshPointNormals() = default;
vtkmTriangleMeshPointNormals::~vtkmTriangleMeshPointNormals() = default;

//------------------------------------------------------------------------------
int vtkmTriangleMeshPointNormals::RequestData(
  vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  // get the info objects
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkPolyData* input = vtkPolyData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData* output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // check if polydata is in supported format
  if (input->GetVerts()->GetNumberOfCells() != 0 || input->GetLines()->GetNumberOfCells() != 0 ||
    input->GetStrips()->GetNumberOfCells() != 0 ||
    (input->GetPolys()->GetNumberOfConnectivityIds() % 3) != 0)
  {
    vtkErrorMacro(<< "This filter only works with polydata containing just triangles.");
    return 0;
  }

  try
  {
    // convert the input dataset to a vtkm::cont::DataSet
    auto in = tovtkm::Convert(input, tovtkm::FieldsFlag::None);

    vtkm::filter::vector_analysis::SurfaceNormals filter;
    filter.SetGenerateCellNormals(false);
    filter.SetNormalizeCellNormals(false);
    filter.SetGeneratePointNormals(true);
    filter.SetPointNormalsName("Normals");
    auto result = filter.Execute(in);

    if (!fromvtkm::Convert(result, output, input))
    {
      vtkErrorMacro(<< "Unable to convert VTKm DataSet back to VTK");
      return 0;
    }
  }
  catch (const vtkm::cont::Error& e)
  {
    if (this->ForceVTKm)
    {
      vtkErrorMacro(<< "VTK-m error: " << e.GetMessage());
      return 0;
    }
    else
    {
      vtkWarningMacro(<< "VTK-m error: " << e.GetMessage()
                      << "Falling back to vtkTriangleMeshPointNormals");
      return this->Superclass::RequestData(request, inputVector, outputVector);
    }
  }

  vtkSmartPointer<vtkDataArray> pointNormals = output->GetPointData()->GetArray("Normals");

  output->GetPointData()->CopyNormalsOff();
  output->GetPointData()->PassData(input->GetPointData());
  output->GetCellData()->CopyNormalsOff();
  output->GetCellData()->PassData(input->GetPointData());

  if (pointNormals)
  {
    output->GetPointData()->SetNormals(pointNormals);
  }

  return 1;
}
VTK_ABI_NAMESPACE_END
