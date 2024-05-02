// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
#include "vtkOpenXRInteractorStyle.h"

#include "vtkObjectFactory.h"
#include "vtkOpenXRRenderWindowInteractor.h"

// Map controller inputs to interaction states
VTK_ABI_NAMESPACE_BEGIN
vtkStandardNewMacro(vtkOpenXRInteractorStyle);

//------------------------------------------------------------------------------
void vtkOpenXRInteractorStyle::SetupActions(vtkRenderWindowInteractor* iren)
{
  vtkOpenXRRenderWindowInteractor* oiren = vtkOpenXRRenderWindowInteractor::SafeDownCast(iren);

  if (oiren)
  {
    oiren->AddAction("elevation", vtkCommand::Elevation3DEvent);
    oiren->AddAction("movement", vtkCommand::ViewerMovement3DEvent);
    oiren->AddAction("nextcamerapose", vtkCommand::NextPose3DEvent);
    oiren->AddAction("positionprop", vtkCommand::PositionProp3DEvent);
    oiren->AddAction("showmenu", vtkCommand::Menu3DEvent);
    oiren->AddAction("startelevation", vtkCommand::Elevation3DEvent);
    oiren->AddAction("startmovement", vtkCommand::ViewerMovement3DEvent);
    oiren->AddAction("triggeraction", vtkCommand::Select3DEvent);
  }
}
VTK_ABI_NAMESPACE_END
