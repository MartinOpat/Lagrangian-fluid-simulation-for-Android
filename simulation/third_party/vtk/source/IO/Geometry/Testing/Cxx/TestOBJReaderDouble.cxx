// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
// .NAME Test of vtkOBJReader to read in double precision points
// .SECTION Description
//

#include <vtkActor.h>
#include <vtkDebugLeaks.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkOBJReader.h>
#include <vtkPolyDataMapper.h>
#include <vtkRegressionTestImage.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkTestUtilities.h>

//------------------------------------------------------------------------------
int TestOBJReaderDouble(int argc, char* argv[])
{
  // Create the reader.
  char* fname = vtkTestUtilities::ExpandDataFileName(argc, argv, "Data/obj_double_sphere.obj");
  vtkNew<vtkOBJReader> reader;
  reader->SetFileName(fname);
  reader->Update();
  delete[] fname;

  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(reader->GetOutputPort());

  // add mapper and texture in an actor
  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);

  // Standard rendering classes
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renWin;
  renWin->AddRenderer(renderer);

  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin);

  // set up the view
  renWin->SetSize(300, 300);

  renderer->AddActor(actor);
  renderer->ResetCamera();

  renWin->Render();

  int retVal = vtkRegressionTestImage(renWin);
  if (retVal == vtkRegressionTester::DO_INTERACTOR)
  {
    iren->Start();
  }

  return !retVal;
}
