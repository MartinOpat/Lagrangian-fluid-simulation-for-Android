// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-FileCopyrightText: Copyright 2008 Sandia Corporation
// SPDX-License-Identifier: LicenseRef-BSD-3-Clause-Sandia-USGov

#include <vtkFixedWidthTextReader.h>
#include <vtkIOStream.h>
#include <vtkNew.h>
#include <vtkStringArray.h>
#include <vtkTable.h>
#include <vtkTestErrorObserver.h>
#include <vtkTestUtilities.h>
#include <vtkVariant.h>
#include <vtkVariantArray.h>

int TestFixedWidthTextReader(int argc, char* argv[])
{
  std::cout << "### Pass 1: No headers, field width 10, do not strip whitespace" << std::endl;

  vtkIdType i, j;
  char* filename = vtkTestUtilities::ExpandDataFileName(argc, argv, "Data/fixedwidth.txt");

  std::cout << "Filename: " << filename << std::endl;

  vtkNew<vtkTest::ErrorObserver> errorObserver1;

  vtkFixedWidthTextReader* reader = vtkFixedWidthTextReader::New();
  reader->SetHaveHeaders(false);
  reader->SetFieldWidth(10);
  reader->StripWhiteSpaceOff();
  reader->SetFileName(filename);
  reader->SetTableErrorObserver(errorObserver1);
  reader->Update();
  int status = errorObserver1->CheckErrorMessage(
    "Incorrect number of tuples in SetRow. Expected 4, but got 6");
  std::cout << "Printing reader info..." << std::endl;
  reader->Print(std::cout);

  vtkTable* table = reader->GetOutput();

  std::cout << "FixedWidth text file has " << table->GetNumberOfRows() << " rows" << std::endl;
  std::cout << "FixedWidth text file has " << table->GetNumberOfColumns() << " columns"
            << std::endl;
  std::cout << "Column names: " << std::endl;

  for (i = 0; i < table->GetNumberOfColumns(); ++i)
  {
    std::cout << "\tColumn " << i << ": " << table->GetColumn(i)->GetName() << std::endl;
  }

  std::cout << "Table contents:" << std::endl;

  for (i = 0; i < table->GetNumberOfRows(); ++i)
  {
    vtkVariantArray* row = table->GetRow(i);

    for (j = 0; j < row->GetNumberOfTuples(); ++j)
    {
      std::cout << "Row " << i << " column " << j << ": ";

      vtkVariant value = row->GetValue(j);
      if (!value.IsValid())
      {
        std::cout << "invalid value" << std::endl;
      }
      else
      {
        std::cout << "type " << value.GetTypeAsString() << " value " << value.ToString()
                  << std::endl;
      }
    }
  }

  reader->Delete();
  delete[] filename;

  reader = vtkFixedWidthTextReader::New();
  filename = vtkTestUtilities::ExpandDataFileName(argc, argv, "Data/fixedwidth.txt");

  reader->HaveHeadersOn();
  reader->SetFieldWidth(10);
  reader->StripWhiteSpaceOn();
  reader->SetFileName(filename);
  reader->SetTableErrorObserver(errorObserver1);
  reader->Update();
  status += errorObserver1->CheckErrorMessage(
    "Incorrect number of tuples in SetRow. Expected 4, but got 6");
  table = reader->GetOutput();

  std::cout << std::endl << "### Test 2: headers, field width 10, strip whitespace" << std::endl;

  std::cout << "Printing reader info..." << std::endl;
  reader->Print(std::cout);

  std::cout << "FixedWidth text file has " << table->GetNumberOfRows() << " rows" << std::endl;
  std::cout << "FixedWidth text file has " << table->GetNumberOfColumns() << " columns"
            << std::endl;
  std::cout << "Column names: " << std::endl;
  for (i = 0; i < table->GetNumberOfColumns(); ++i)
  {
    std::cout << "\tColumn " << i << ": " << table->GetColumn(i)->GetName() << std::endl;
  }

  std::cout << "Table contents:" << std::endl;

  for (i = 0; i < table->GetNumberOfRows(); ++i)
  {
    vtkVariantArray* row = table->GetRow(i);

    for (j = 0; j < row->GetNumberOfTuples(); ++j)
    {
      std::cout << "Row " << i << " column " << j << ": ";

      vtkVariant value = row->GetValue(j);
      if (!value.IsValid())
      {
        std::cout << "invalid value" << std::endl;
      }
      else
      {
        std::cout << "type " << value.GetTypeAsString() << " value " << value.ToString()
                  << std::endl;
      }
    }
  }

  reader->Delete();
  delete[] filename;

  if (status != 0)
  {
    return EXIT_FAILURE;
  }

  return 0;
}
