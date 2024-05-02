// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
#include "vtkBackgroundColorMonitor.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include <cmath>
#include <cstring>

//------------------------------------------------------------------------------
VTK_ABI_NAMESPACE_BEGIN
vtkStandardNewMacro(vtkBackgroundColorMonitor);

//------------------------------------------------------------------------------
vtkBackgroundColorMonitor::vtkBackgroundColorMonitor()
  : UpTime(0)
  , Gradient(false)
{
  memset(this->Color1, 0, 3 * sizeof(double));
  memset(this->Color2, 0, 3 * sizeof(double));
}

//------------------------------------------------------------------------------
bool vtkBackgroundColorMonitor::StateChanged(vtkRenderer* ren)
{
  unsigned int oldUpTime = this->UpTime;
  this->Update(ren);
  return oldUpTime != this->UpTime;
}

//------------------------------------------------------------------------------
void vtkBackgroundColorMonitor::Update(vtkRenderer* ren)
{
  // update colors
  double* color1 = ren->GetBackground();
  double* color2 = ren->GetBackground2();
  bool changed = false;
  for (int i = 0; i < 3; ++i)
  {
    if ((this->Color1[i] != color1[i]) || (this->Color2[i] != color2[i]))
    {
      changed = true;
    }
    this->Color1[i] = color1[i];
    this->Color2[i] = color2[i];
  }
  // update gradient flag
  bool grad = ren->GetGradientBackground();
  if (this->Gradient != grad)
  {
    changed = true;
  }
  this->Gradient = grad;
  // update mtime
  if (changed)
  {
    this->UpTime += 1;
  }
}

//------------------------------------------------------------------------------
void vtkBackgroundColorMonitor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Gradient=" << this->Gradient << endl;
  os << indent << "Color1=";
  for (int q = 0; q < 3; ++q)
  {
    os << this->Color1[q] << " ";
  }
  os << endl;
  os << indent << "Color2=";
  for (int q = 0; q < 3; ++q)
  {
    os << this->Color2[q] << " ";
  }
  os << endl;
  os << indent << "UpTime=" << this->UpTime << endl;
}
VTK_ABI_NAMESPACE_END
