// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause

#include "vtkPropItem.h"

#include "vtkContextScene.h"
#include "vtkObjectFactory.h"
#include "vtkProp.h"
#include "vtkProp3D.h"
#include "vtkRenderer.h"

VTK_ABI_NAMESPACE_BEGIN
vtkObjectFactoryNewMacro(vtkPropItem);
vtkCxxSetObjectMacro(vtkPropItem, PropObject, vtkProp);

//------------------------------------------------------------------------------
vtkPropItem::vtkPropItem()
  : PropObject(nullptr)
{
}

//------------------------------------------------------------------------------
vtkPropItem::~vtkPropItem()
{
  this->SetPropObject(nullptr);
}

//------------------------------------------------------------------------------
void vtkPropItem::UpdateTransforms()
{
  vtkErrorMacro(<< "Missing override in the rendering backend. Some items "
                   "may be rendered incorrectly.");
}

//------------------------------------------------------------------------------
void vtkPropItem::ResetTransforms()
{
  vtkErrorMacro(<< "Missing override in the rendering backend. Some items "
                   "may be rendered incorrectly.");
}

//------------------------------------------------------------------------------
bool vtkPropItem::Paint(vtkContext2D*)
{
  if (!this->PropObject)
  {
    return false;
  }

  this->UpdateTransforms();

  int result = this->PropObject->RenderOpaqueGeometry(this->Scene->GetRenderer());
  if (this->PropObject->HasTranslucentPolygonalGeometry())
  {
    result += this->PropObject->RenderTranslucentPolygonalGeometry(this->Scene->GetRenderer());
  }
  result += this->PropObject->RenderOverlay(this->Scene->GetRenderer());

  this->ResetTransforms();

  return result != 0;
}

//------------------------------------------------------------------------------
void vtkPropItem::ReleaseGraphicsResources()
{
  if (this->PropObject && this->Scene && this->Scene->GetRenderer())
  {
    this->PropObject->ReleaseGraphicsResources(this->Scene->GetRenderer()->GetVTKWindow());
  }
}

//------------------------------------------------------------------------------
void vtkPropItem::PrintSelf(std::ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Prop:";
  if (this->PropObject)
  {
    os << "\n";
    this->PropObject->PrintSelf(os, indent.GetNextIndent());
  }
  else
  {
    os << "(nullptr)\n";
  }
}
VTK_ABI_NAMESPACE_END
