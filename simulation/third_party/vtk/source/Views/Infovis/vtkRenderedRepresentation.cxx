// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-FileCopyrightText: Copyright 2008 Sandia Corporation
// SPDX-License-Identifier: LicenseRef-BSD-3-Clause-Sandia-USGov

#include "vtkRenderedRepresentation.h"

#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkProp.h"
#include "vtkRenderView.h"
#include "vtkRenderer.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkSmartPointer.h"

#include <vector>

VTK_ABI_NAMESPACE_BEGIN
vtkStandardNewMacro(vtkRenderedRepresentation);

class vtkRenderedRepresentation::Internals
{
public:
  // Convenience vectors for storing props to add/remove until the next render,
  // where they are added/removed by PrepareForRendering().
  std::vector<vtkSmartPointer<vtkProp>> PropsToAdd;
  std::vector<vtkSmartPointer<vtkProp>> PropsToRemove;
};

vtkRenderedRepresentation::vtkRenderedRepresentation()
{
  this->Implementation = new Internals();
  this->LabelRenderMode = vtkRenderView::FREETYPE;
}

vtkRenderedRepresentation::~vtkRenderedRepresentation()
{
  delete this->Implementation;
}

void vtkRenderedRepresentation::AddPropOnNextRender(vtkProp* p)
{
  this->Implementation->PropsToAdd.emplace_back(p);
}

void vtkRenderedRepresentation::RemovePropOnNextRender(vtkProp* p)
{
  this->Implementation->PropsToRemove.emplace_back(p);
}

void vtkRenderedRepresentation::PrepareForRendering(vtkRenderView* view)
{
  // Add props scheduled to be added on next render.
  for (size_t i = 0; i < this->Implementation->PropsToAdd.size(); ++i)
  {
    view->GetRenderer()->AddViewProp(this->Implementation->PropsToAdd[i]);
  }
  this->Implementation->PropsToAdd.clear();

  // Remove props scheduled to be removed on next render.
  for (size_t i = 0; i < this->Implementation->PropsToRemove.size(); ++i)
  {
    view->GetRenderer()->RemoveViewProp(this->Implementation->PropsToRemove[i]);
  }
  this->Implementation->PropsToRemove.clear();
}

std::string vtkRenderedRepresentation::GetHoverString(vtkView* view, vtkProp* prop, vtkIdType cell)
{
  vtkSmartPointer<vtkSelection> cellSelect = vtkSmartPointer<vtkSelection>::New();
  vtkSmartPointer<vtkSelectionNode> cellNode = vtkSmartPointer<vtkSelectionNode>::New();
  cellNode->GetProperties()->Set(vtkSelectionNode::PROP(), prop);
  cellNode->SetFieldType(vtkSelectionNode::CELL);
  cellNode->SetContentType(vtkSelectionNode::INDICES);
  vtkSmartPointer<vtkIdTypeArray> idArr = vtkSmartPointer<vtkIdTypeArray>::New();
  idArr->InsertNextValue(cell);
  cellNode->SetSelectionList(idArr);
  cellSelect->AddNode(cellNode);
  vtkSelection* converted = this->ConvertSelection(view, cellSelect);
  std::string text = this->GetHoverStringInternal(converted);
  if (converted != cellSelect)
  {
    converted->Delete();
  }
  return text;
}

void vtkRenderedRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "LabelRenderMode: " << this->LabelRenderMode << endl;
}
VTK_ABI_NAMESPACE_END
