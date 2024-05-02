// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause

#include "vtkImageData.h"
#include "vtkMolecule.h"
#include "vtkNew.h"
#include "vtkProgrammableElectronicData.h"

#define CHECK_MO(num)                                                                              \
  do                                                                                               \
  {                                                                                                \
    if (ed->GetMO(num) != mo##num)                                                                 \
    {                                                                                              \
      cerr << "MO number " << (num) << " has changed since being set: "                            \
           << "Expected @" << mo##num << ", got @" << ed->GetMO(num) << ".\n";                     \
      return EXIT_FAILURE;                                                                         \
    }                                                                                              \
  } while (false)

int TestProgrammableElectronicData(int, char*[])
{
  vtkNew<vtkMolecule> mol;
  vtkNew<vtkProgrammableElectronicData> ed;

  vtkNew<vtkImageData> mo1;
  vtkNew<vtkImageData> mo2;
  vtkNew<vtkImageData> mo3;
  vtkNew<vtkImageData> mo4;
  vtkNew<vtkImageData> mo5;
  vtkNew<vtkImageData> mo6;
  vtkNew<vtkImageData> mo7;
  vtkNew<vtkImageData> mo8;
  vtkNew<vtkImageData> density;

  ed->SetMO(1, mo1);
  ed->SetMO(2, mo2);
  ed->SetMO(3, mo3);
  ed->SetMO(4, mo4);
  ed->SetMO(5, mo5);
  ed->SetMO(6, mo6);
  ed->SetMO(7, mo7);
  ed->SetMO(8, mo8);
  ed->SetElectronDensity(density);

  CHECK_MO(1);
  CHECK_MO(2);
  CHECK_MO(3);
  CHECK_MO(4);
  CHECK_MO(5);
  CHECK_MO(6);
  CHECK_MO(7);
  CHECK_MO(8);

  if (ed->GetElectronDensity() != density)
  {
    cerr << "Electron density has changed since being set.";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
