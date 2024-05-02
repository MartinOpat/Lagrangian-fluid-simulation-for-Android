// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
#include "vtkScalarBarWidget.h"

#include "vtkCallbackCommand.h"
#include "vtkCoordinate.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkScalarBarActor.h"
#include "vtkScalarBarRepresentation.h"
#include "vtkWidgetCallbackMapper.h"
#include "vtkWidgetEvent.h"

VTK_ABI_NAMESPACE_BEGIN
vtkStandardNewMacro(vtkScalarBarWidget);

//------------------------------------------------------------------------------
vtkScalarBarWidget::vtkScalarBarWidget()
{
  this->Selectable = 0;
  this->Repositionable = 1;

  // Override the subclasses callback to handle the Repositionable flag.
  this->CallbackMapper->SetCallbackMethod(
    vtkCommand::MouseMoveEvent, vtkWidgetEvent::Move, this, vtkScalarBarWidget::MoveAction);
}

//------------------------------------------------------------------------------
vtkScalarBarWidget::~vtkScalarBarWidget() = default;

//------------------------------------------------------------------------------
void vtkScalarBarWidget::SetRepresentation(vtkScalarBarRepresentation* rep)
{
  this->SetWidgetRepresentation(rep);
}

//------------------------------------------------------------------------------
void vtkScalarBarWidget::SetScalarBarActor(vtkScalarBarActor* actor)
{
  vtkScalarBarRepresentation* rep = this->GetScalarBarRepresentation();
  if (!rep)
  {
    this->CreateDefaultRepresentation();
    rep = this->GetScalarBarRepresentation();
  }

  if (rep->GetScalarBarActor() != actor)
  {
    rep->SetScalarBarActor(actor);
    this->Modified();
  }
}

//------------------------------------------------------------------------------
vtkScalarBarActor* vtkScalarBarWidget::GetScalarBarActor()
{
  vtkScalarBarRepresentation* rep = this->GetScalarBarRepresentation();
  if (!rep)
  {
    this->CreateDefaultRepresentation();
    rep = this->GetScalarBarRepresentation();
  }

  return rep->GetScalarBarActor();
}

//------------------------------------------------------------------------------
void vtkScalarBarWidget::CreateDefaultRepresentation()
{
  if (!this->WidgetRep)
  {
    vtkScalarBarRepresentation* rep = vtkScalarBarRepresentation::New();
    this->SetRepresentation(rep);
    rep->Delete();
  }
}

//------------------------------------------------------------------------------
void vtkScalarBarWidget::SetCursor(int cState)
{
  if (!this->Repositionable && !this->Selectable && cState == vtkBorderRepresentation::Inside)
  {
    // Don't have a special cursor for the inside if we cannot reposition.
    this->RequestCursorShape(VTK_CURSOR_DEFAULT);
  }
  else
  {
    this->Superclass::SetCursor(cState);
  }
}

//------------------------------------------------------------------------------
void vtkScalarBarWidget::MoveAction(vtkAbstractWidget* w)
{
  // The superclass handle most stuff.
  vtkScalarBarWidget::Superclass::MoveAction(w);

  vtkScalarBarWidget* self = reinterpret_cast<vtkScalarBarWidget*>(w);
  vtkScalarBarRepresentation* representation = self->GetScalarBarRepresentation();

  // Handle the case where we suppress widget translation.
  if (!self->Repositionable &&
    (representation->GetInteractionState() == vtkBorderRepresentation::Inside))
  {
    representation->MovingOff();
  }
}

//------------------------------------------------------------------------------
vtkTypeBool vtkScalarBarWidget::GetProcessEvents()
{
  return this->vtkAbstractWidget::GetProcessEvents(); // NOLINT(bugprone-parent-virtual-call)
}

//------------------------------------------------------------------------------
void vtkScalarBarWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Repositionable: " << this->Repositionable << endl;
}
VTK_ABI_NAMESPACE_END
