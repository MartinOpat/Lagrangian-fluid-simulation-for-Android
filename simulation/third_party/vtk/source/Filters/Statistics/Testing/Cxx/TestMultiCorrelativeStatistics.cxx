// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-FileCopyrightText: Copyright 2008 Sandia Corporation
// SPDX-License-Identifier: LicenseRef-BSD-3-Clause-Sandia-USGov
// .SECTION Thanks
// Thanks to Philippe Pebay and David Thompson from Sandia National Laboratories
// for implementing this test.

#include "vtkDataSetAttributes.h"
#include "vtkDoubleArray.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkMultiCorrelativeStatistics.h"
#include "vtkNew.h"
#include "vtkStringArray.h"
#include "vtkTable.h"
#include "vtkUnsignedCharArray.h"

//=============================================================================
int TestMultiCorrelativeStatistics(int, char*[])
{
  int testStatus = 0;

  /* */
  unsigned char ghostArray[] = {
    0, // 1
    0, // 2
    0, // 3
    0, // 4
    0, // 5
    0, // 6
    0, // 7
    0, // 8
    0, // 9
    0, // 10
    0, // 11
    0, // 12
    0, // 13
    0, // 14
    0, // 15
    0, // 16
    0, // 17
    0, // 18
    0, // 19
    0, // 20
    0, // 21
    0, // 22
    0, // 23
    1, // 24
    0, // 25
    0, // 26
    0, // 27
    0, // 28
    0, // 29
    0, // 30
    0, // 31
    0, // 32
    0, // 33
  };

  /* */
  double mingledData[] = {
    46, 45,   //
    47, 49,   //
    46, 47,   //
    46, 46,   //
    47, 46,   //
    47, 49,   //
    49, 49,   //
    47, 45,   //
    50, 50,   //
    46, 46,   //
    51, 50,   //
    48, 48,   //
    52, 54,   //
    48, 47,   //
    52, 52,   //
    49, 49,   //
    53, 54,   //
    50, 50,   //
    53, 54,   //
    50, 52,   //
    53, 53,   //
    50, 51,   //
    54, 54,   //
    999, 999, // 24 (ghosts)
    49, 49,   //
    52, 52,   //
    50, 51,   //
    52, 52,   //
    49, 47,   //
    48, 48,   //
    48, 50,   //
    46, 48,   //
    47, 47    //
  };
  int nVals = 33;

  const char m0Name[] = "M0";
  vtkDoubleArray* dataset1Arr = vtkDoubleArray::New();
  dataset1Arr->SetNumberOfComponents(1);
  dataset1Arr->SetName(m0Name);

  const char m1Name[] = "M1";
  vtkDoubleArray* dataset2Arr = vtkDoubleArray::New();
  dataset2Arr->SetNumberOfComponents(1);
  dataset2Arr->SetName(m1Name);

  const char m2Name[] = "M2";
  vtkDoubleArray* dataset3Arr = vtkDoubleArray::New();
  dataset3Arr->SetNumberOfComponents(1);
  dataset3Arr->SetName(m2Name);

  vtkNew<vtkUnsignedCharArray> ghosts;
  ghosts->SetName(vtkDataSetAttributes::GhostArrayName());

  for (int i = 0; i < nVals; ++i)
  {
    int ti = i << 1;
    dataset1Arr->InsertNextValue(mingledData[ti]);
    dataset2Arr->InsertNextValue(mingledData[ti + 1]);
    dataset3Arr->InsertNextValue(i != 12 ? -1. : -1.001);
    ghosts->InsertNextValue(ghostArray[i]);
  }

  vtkTable* datasetTable = vtkTable::New();
  datasetTable->AddColumn(dataset1Arr);
  dataset1Arr->Delete();
  datasetTable->AddColumn(dataset2Arr);
  dataset2Arr->Delete();
  datasetTable->AddColumn(dataset3Arr);
  dataset3Arr->Delete();
  datasetTable->AddColumn(ghosts);
  datasetTable->GetRowData()->SetGhostsToSkip(1);

  // Set multi-correlative statistics algorithm and its input data port
  vtkMultiCorrelativeStatistics* mcs = vtkMultiCorrelativeStatistics::New();

  // First verify that absence of input does not cause trouble
  cout << "## Verifying that absence of input does not cause trouble... ";
  mcs->Update();
  cout << "done.\n";

  // Prepare first test with data
  mcs->SetInputData(vtkStatisticsAlgorithm::INPUT_DATA, datasetTable);

  datasetTable->Delete();

  // Select Column Pairs of Interest ( Learn Mode )
  mcs->SetColumnStatus(m0Name, 1);
  mcs->SetColumnStatus(m1Name, 1);
  mcs->RequestSelectedColumns();
  mcs->ResetAllColumnStates();
  mcs->SetColumnStatus(m0Name, 1);
  mcs->SetColumnStatus(m1Name, 1);
  mcs->SetColumnStatus(m2Name, 1);
  mcs->SetColumnStatus(m2Name, 0);
  mcs->SetColumnStatus(m2Name, 1);
  mcs->RequestSelectedColumns();
  mcs->RequestSelectedColumns(); // Try a duplicate entry. This should have no effect.
  mcs->SetColumnStatus(m0Name, 0);
  mcs->SetColumnStatus(m2Name, 0);
  mcs->SetColumnStatus("Metric 3",
    1); // An invalid name. This should result in a request for metric 1's self-correlation.
  // mcs->RequestSelectedColumns(); will get called in RequestData()

  // Test Learn Mode
  mcs->SetLearnOption(true);
  mcs->SetDeriveOption(true);
  mcs->SetAssessOption(false);

  mcs->Update();
  vtkMultiBlockDataSet* outputMetaDS = vtkMultiBlockDataSet::SafeDownCast(
    mcs->GetOutputDataObject(vtkStatisticsAlgorithm::OUTPUT_MODEL));

  cout << "## Calculated the following statistics for data set:\n";
  for (unsigned int b = 0; b < outputMetaDS->GetNumberOfBlocks(); ++b)
  {
    vtkTable* outputMeta = vtkTable::SafeDownCast(outputMetaDS->GetBlock(b));

    if (b == 0)
    {
      cout << "Primary Statistics\n";
    }
    else
    {
      cout << "Derived Statistics " << (b - 1) << "\n";
    }

    outputMeta->Dump();
  }

  // Test Assess Mode
  vtkMultiBlockDataSet* paramsTables = vtkMultiBlockDataSet::New();
  paramsTables->ShallowCopy(outputMetaDS);

  mcs->SetInputData(vtkStatisticsAlgorithm::INPUT_MODEL, paramsTables);
  paramsTables->Delete();

  // Test Assess only (Do not recalculate nor rederive a model)
  mcs->SetLearnOption(false);
  mcs->SetDeriveOption(false);
  mcs->SetAssessOption(true);
  mcs->Update();

  vtkTable* outputData = mcs->GetOutput();
  outputData->Dump();

  // Threshold for outlier detection
  double threshold = 4.;
  int nOutliers = 0;
  int tableIdx[] = { 0, 1, 3 };

  cout << "## Searching for outliers such that " << outputData->GetColumnName(tableIdx[2]) << " > "
       << threshold << "\n";

  cout << "   Found the following outliers:\n";
  for (int i = 0; i < 3; ++i)
  {
    cout << "   " << outputData->GetColumnName(tableIdx[i]);
  }
  cout << "\n";

  for (vtkIdType r = 0; r < outputData->GetNumberOfRows(); ++r)
  {
    if (outputData->GetValue(r, tableIdx[2]).ToDouble() > threshold)
    {
      ++nOutliers;

      for (int i = 0; i < 3; ++i)
      {
        cout << "     " << outputData->GetValue(r, tableIdx[i]).ToString() << "    ";
      }
      cout << "\n";
    }
  }

  if (nOutliers != 3)
  {
    vtkGenericWarningMacro("Expected 3 outliers, found " << nOutliers << ".");
    testStatus = 1;
  }

  mcs->Delete();

  return testStatus;
}
