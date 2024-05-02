// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
#include "vtkCGNSReader.h"
#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkDataArray.h"
#include "vtkDataSet.h"
#include "vtkInformation.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkPointSet.h"
#include "vtkTestUtilities.h"
#include "vtkTimerLog.h"

#include <string>

#define vtk_assert(x)                                                                              \
  do                                                                                               \
  {                                                                                                \
    if (!(x))                                                                                      \
    {                                                                                              \
      cerr << "On line " << __LINE__ << " ERROR: Condition FAILED!! : " << #x << endl;             \
      return EXIT_FAILURE;                                                                         \
    }                                                                                              \
  } while (false)

int TestCGNSReaderMeshCaching(int argc, char* argv[])
{
  float cold_timing, hot_timing;
  char* fname = vtkTestUtilities::ExpandDataFileName(argc, argv, "Data/test_node_and_cell.cgns");
  vtkNew<vtkCGNSReader> reader;
  reader->SetFileName(fname);
  delete[] fname;

  reader->UpdateInformation();
  reader->EnableAllCellArrays();
  reader->EnableAllPointArrays();
  // Enable caching to be tested
  reader->CacheMeshOn();
  reader->CacheConnectivityOn();

  vtkNew<vtkTimerLog> timer;
  timer->StartTimer();
  reader->Update();
  timer->StopTimer();
  cold_timing = timer->GetElapsedTime();

  // Do some basic checks
  vtkMultiBlockDataSet* mb = reader->GetOutput();
  vtkPointSet* ds =
    vtkPointSet::SafeDownCast(vtkMultiBlockDataSet::SafeDownCast(mb->GetBlock(0))->GetBlock(0));
  vtk_assert(ds != nullptr);
  vtk_assert(ds->GetCellData()->GetArray("Pressure") != nullptr);
  vtkDataArray* da = ds->GetPoints()->GetData();

  reader->DisableAllCellArrays();
  timer->StartTimer();
  reader->Update();
  timer->StopTimer();
  hot_timing = timer->GetElapsedTime();
  //
  mb = reader->GetOutput();
  ds = vtkPointSet::SafeDownCast(vtkMultiBlockDataSet::SafeDownCast(mb->GetBlock(0))->GetBlock(0));
  vtk_assert(ds != nullptr);
  vtkDataArray* db = ds->GetPoints()->GetData();
  vtk_assert(db != nullptr);

  // Check Mesh Data pointer did not change between loadings
  vtk_assert(da == db);
  // Check that caching mesh implies lower loading time
  // vtk_assert(hot_timing < cold_timing);
  cout << "Expected timings: " << hot_timing << " < " << cold_timing << endl;

  return EXIT_SUCCESS;
}
