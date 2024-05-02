// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
#if VTK_MODULE_ENABLE_VTK_ParallelMPI
#include "vtkMPIController.h"
#else
#include "vtkDummyController.h"
#endif

#include "vtkNew.h"
#include "vtkTestUtilities.h"
#include "vtkXMLMultiBlockDataReader.h"

int TestXMLReaderChangingBlocksOverTime(int argc, char* argv[])
{
#if VTK_MODULE_ENABLE_VTK_ParallelMPI
  vtkNew<vtkMPIController> contr;
#else
  vtkNew<vtkDummyController> contr;
#endif
  contr->Initialize(&argc, &argv);
  vtkMultiProcessController::SetGlobalController(contr);

  char* root = vtkTestUtilities::GetDataRoot(argc, argv);
  const std::string dataRoot = root;
  delete[] root;
  std::string filename(dataRoot);
  filename += "/Data/ChangingBlocksOverTime/wavelet_0.vtm";

  vtkNew<vtkXMLMultiBlockDataReader> reader;
  reader->SetFileName(filename.c_str());
  reader->UpdatePiece(contr->GetLocalProcessId(), contr->GetNumberOfProcesses(), 0);

  filename = dataRoot + "/Data/ChangingBlocksOverTime/wavelet_1.vtm";
  reader->SetFileName(filename.c_str());
  reader->UpdatePiece(contr->GetLocalProcessId(), contr->GetNumberOfProcesses(), 0);

  vtkMultiProcessController::SetGlobalController(nullptr);
  contr->Finalize();
  return EXIT_SUCCESS;
}
