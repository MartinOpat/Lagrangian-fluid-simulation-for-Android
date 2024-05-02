// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
#include "vtkNewickTreeWriter.h"

#include "vtkDataSetAttributes.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkTree.h"

VTK_ABI_NAMESPACE_BEGIN
vtkStandardNewMacro(vtkNewickTreeWriter);

//------------------------------------------------------------------------------
vtkNewickTreeWriter::vtkNewickTreeWriter()
{
  this->SetFileTypeToASCII();

  this->EdgeWeightArrayName = "weight";
  this->NodeNameArrayName = "node name";

  this->EdgeWeightArray = nullptr;
  this->NodeNameArray = nullptr;
}

//------------------------------------------------------------------------------
void vtkNewickTreeWriter::WriteData()
{
  vtkDebugMacro(<< "Writing vtk tree data...");

  vtkTree* const input = this->GetInput();

  this->EdgeWeightArray = input->GetEdgeData()->GetAbstractArray(this->EdgeWeightArrayName.c_str());

  this->NodeNameArray = input->GetVertexData()->GetAbstractArray(this->NodeNameArrayName.c_str());

  ostream* fp = this->OpenVTKFile();
  if (!fp)
  {
    vtkErrorMacro("Failed to open output stream");
    return;
  }

  this->WriteVertex(fp, input, input->GetRoot());

  // the tree ends with a semi-colon
  *fp << ";";

  this->CloseVTKFile(fp);
}

//------------------------------------------------------------------------------
void vtkNewickTreeWriter::WriteVertex(ostream* fp, vtkTree* input, vtkIdType vertex)
{
  vtkIdType numChildren = input->GetNumberOfChildren(vertex);
  if (numChildren > 0)
  {
    *fp << "(";
    for (vtkIdType child = 0; child < numChildren; ++child)
    {
      this->WriteVertex(fp, input, input->GetChild(vertex, child));
      if (child != numChildren - 1)
      {
        *fp << ",";
      }
    }
    *fp << ")";
  }

  if (this->NodeNameArray)
  {
    std::string name = this->NodeNameArray->GetVariantValue(vertex).ToString();
    if (!name.empty())
    {
      *fp << name;
    }
  }

  if (this->EdgeWeightArray)
  {
    vtkIdType parent = input->GetParent(vertex);
    if (parent != -1)
    {
      vtkIdType edge = input->GetEdgeId(parent, vertex);
      if (edge != -1)
      {
        double weight = this->EdgeWeightArray->GetVariantValue(edge).ToDouble();
        *fp << ":" << weight;
      }
    }
  }
}

//------------------------------------------------------------------------------
int vtkNewickTreeWriter::FillInputPortInformation(int, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkTree");
  return 1;
}

//------------------------------------------------------------------------------
vtkTree* vtkNewickTreeWriter::GetInput()
{
  return vtkTree::SafeDownCast(this->Superclass::GetInput());
}

//------------------------------------------------------------------------------
vtkTree* vtkNewickTreeWriter::GetInput(int port)
{
  return vtkTree::SafeDownCast(this->Superclass::GetInput(port));
}

//------------------------------------------------------------------------------
void vtkNewickTreeWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "EdgeWeightArrayName: " << this->EdgeWeightArrayName << endl;
  os << indent << "NodeNameArrayName: " << this->NodeNameArrayName << endl;
}
VTK_ABI_NAMESPACE_END
