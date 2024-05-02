// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-FileCopyrightText: Copyright 2016 Menno Deij - van Rijswijk (MARIN)
// SPDX-License-Identifier: BSD-3-Clause
#include "vtkTecplotTableReader.h"
#include "vtkCommand.h"
#include "vtkDataSetAttributes.h"
#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTable.h"

#include "vtkTextCodec.h"
#include "vtkTextCodecFactory.h"
#include "vtksys/FStream.hxx"

#include <vtk_utf8.h>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <cctype>

////////////////////////////////////////////////////////////////////////////////
// DelimitedTextIterator

/// Output iterator object that parses a stream of Unicode characters into records and
/// fields, inserting them into a vtkTable. Based on the iterator from
/// vtkIOInfoVis::DelimitedTextReader but tailored to Tecplot table files

VTK_ABI_NAMESPACE_BEGIN
namespace
{

class DelimitedTextIterator : public vtkTextCodec::OutputIterator
{
public:
  typedef std::forward_iterator_tag iterator_category;
  typedef vtkTypeUInt32 value_type;
  typedef std::string::difference_type difference_type;
  typedef value_type* pointer;
  typedef value_type& reference;

  DelimitedTextIterator(vtkTable* outputTable, vtkIdType maxRecords, vtkIdType headerLines,
    vtkIdType columnHeadersOnLine, vtkIdType skipColumnNames)
    : MaxRecords(maxRecords)
    // first two lines are title + column names
    , MaxRecordIndex(maxRecords + headerLines)
    , WhiteSpaceOnlyString(true)
    , OutputTable(outputTable)
    , CurrentRecordIndex(0)
    , CurrentFieldIndex(0)
    , HeaderLines(headerLines)
    , ColumnNamesOnLine(columnHeadersOnLine)
    , SkipColumnNames(skipColumnNames)
    , RecordAdjacent(true)
    , MergeConsDelims(true)
    , ProcessEscapeSequence(false)
    , UseStringDelimiter(true)
    , WithinString(0)
  {
    // how records (e.g. lines) are separated
    RecordDelimiters.insert('\n');
    RecordDelimiters.insert('\r');

    // how fields (e.g. entries) are separated
    FieldDelimiters.insert(' ');
    FieldDelimiters.insert('\t');

    // how string entries are separated
    StringDelimiters.insert('"');
    StringDelimiters.insert(' ');

    // what is whitespace
    Whitespace.insert(' ');
    Whitespace.insert('\t');
  }

  ~DelimitedTextIterator() override
  {
    // Ensure that all table columns have the same length ...
    for (vtkIdType i = 0; i != this->OutputTable->GetNumberOfColumns(); ++i)
    {
      if (this->OutputTable->GetColumn(i)->GetNumberOfTuples() !=
        this->OutputTable->GetColumn(0)->GetNumberOfTuples())
      {
        this->OutputTable->GetColumn(i)->Resize(
          this->OutputTable->GetColumn(0)->GetNumberOfTuples());
      }
    }
  }

  // Handle windows files that do not have a carriage return line feed on the last line of the file
  // ...
  void ReachedEndOfInput()
  {
    if (!this->CurrentField.empty())
    {
      std::string::iterator start = this->CurrentField.begin();
      std::string::iterator end = this->CurrentField.end();
      std::string::size_type size = utf8::distance(start, end);

      std::string::iterator iterator = start;
      utf8::advance(iterator, size - 1, end);
      vtkTypeUInt32 value = utf8::next(iterator, end);
      // if the last character is not a CR/LF add a column
      if (!this->RecordDelimiters.count(value) && !this->Whitespace.count(value))
      {
        this->InsertField();
      }
    }
  }

  DelimitedTextIterator& operator=(const vtkTypeUInt32& value) override
  {
    // If we've already read our maximum number of records, we're done ...
    if (this->MaxRecords && this->CurrentRecordIndex == this->MaxRecordIndex)
    {
      return *this;
    }

    // Strip adjacent record delimiters and whitespace...
    if (this->RecordAdjacent &&
      (this->RecordDelimiters.count(value) || this->Whitespace.count(value)))
    {
      return *this;
    }
    else
    {
      this->RecordAdjacent = false;
    }

    // Look for record delimiters ...
    if (this->RecordDelimiters.count(value))
    {
      // keep skipping until column names line
      if (this->CurrentRecordIndex < ColumnNamesOnLine)
      {
        this->CurrentRecordIndex += 1;
        return *this;
      }

      this->InsertField();
      this->CurrentRecordIndex += 1;
      this->CurrentFieldIndex = 0;
      this->CurrentField.clear();
      this->RecordAdjacent = true;
      this->WithinString = 0;
      this->WhiteSpaceOnlyString = true;
      return *this;
    }

    if (this->CurrentRecordIndex < ColumnNamesOnLine)
    {
      return *this; // keep skipping until column names line
    }

    // Look for field delimiters unless we're in a string ...
    if (!this->WithinString && this->FieldDelimiters.count(value))
    {
      // Handle special case of merging consecutive delimiters ...
      if (!(this->CurrentField.empty() && this->MergeConsDelims))
      {
        if (!(this->CurrentFieldIndex < SkipColumnNames &&
              this->CurrentRecordIndex == ColumnNamesOnLine))
        // if (!(this->CurrentFieldIndex == 0 && this->CurrentRecordIndex == 1))
        {
          this->InsertField();
        }
        this->CurrentFieldIndex += 1;
        this->CurrentField.clear();
      }
      return *this;
    }

    // Check for start of escape sequence ...
    if (!this->ProcessEscapeSequence && this->EscapeDelimiter.count(value))
    {
      this->ProcessEscapeSequence = true;
      return *this;
    }

    // Process escape sequence ...
    if (this->ProcessEscapeSequence)
    {
      std::string curr_char;
      utf8::append(value, std::back_inserter(curr_char));
      if (curr_char == "a")
      {
        this->CurrentField += "\a";
      }
      else if (curr_char == "b")
      {
        this->CurrentField += "\b";
      }
      else if (curr_char == "t")
      {
        this->CurrentField += "\t";
      }
      else if (curr_char == "n")
      {
        this->CurrentField += "\n";
      }
      else if (curr_char == "v")
      {
        this->CurrentField += "\v";
      }
      else if (curr_char == "f")
      {
        this->CurrentField += "\f";
      }
      else if (curr_char == "r")
      {
        this->CurrentField += "\r";
      }
      else if (curr_char == "\\")
      {
        this->CurrentField += "\\";
      }
      else if (!(curr_char == "0"))
      {
        this->CurrentField += curr_char;
      }
      this->ProcessEscapeSequence = false;
      return *this;
    }

    // Start a string ...
    if (!this->WithinString && this->StringDelimiters.count(value) && this->UseStringDelimiter)
    {
      this->WithinString = value;
      this->CurrentField.clear();
      return *this;
    }

    // End a string ...
    if (this->WithinString && (this->WithinString == value) && this->UseStringDelimiter)
    {
      this->WithinString = 0;
      return *this;
    }

    if (!this->Whitespace.count(value))
    {
      this->WhiteSpaceOnlyString = false;
    }
    // Keep growing the current field ...
    utf8::append(value, std::back_inserter(this->CurrentField));
    return *this;
  }

private:
  void InsertField()
  {
    vtkIdType fieldIndex = this->CurrentFieldIndex;
    if (this->CurrentRecordIndex == ColumnNamesOnLine)
    {
      fieldIndex -= SkipColumnNames;
    }

    if (fieldIndex >= this->OutputTable->GetNumberOfColumns() &&
      ColumnNamesOnLine == this->CurrentRecordIndex)
    {
      vtkDoubleArray* array = vtkDoubleArray::New();

      array->SetName(this->CurrentField.c_str());
      this->OutputTable->AddColumn(array);
      array->Delete();
    }
    else if (fieldIndex < this->OutputTable->GetNumberOfColumns())
    {
      // Handle case where input file has header information ...
      vtkIdType recordIndex;
      recordIndex = this->CurrentRecordIndex - HeaderLines;
      vtkDoubleArray* array =
        vtkArrayDownCast<vtkDoubleArray>(this->OutputTable->GetColumn(fieldIndex));

      vtkStdString str = this->CurrentField;
      bool ok;
      double doubleValue = vtkVariant(str).ToDouble(&ok);
      if (ok)
      {
        array->InsertValue(recordIndex, doubleValue);
      }
      else
      {
        array->InsertValue(recordIndex, std::numeric_limits<double>::quiet_NaN());
      }
    }
  }

  vtkIdType MaxRecords;
  vtkIdType MaxRecordIndex;
  std::set<vtkTypeUInt32> RecordDelimiters;
  std::set<vtkTypeUInt32> FieldDelimiters;
  std::set<vtkTypeUInt32> StringDelimiters;
  std::set<vtkTypeUInt32> Whitespace;
  std::set<vtkTypeUInt32> EscapeDelimiter;

  bool WhiteSpaceOnlyString;
  vtkTable* OutputTable;
  vtkIdType CurrentRecordIndex;
  vtkIdType CurrentFieldIndex;
  std::string CurrentField;

  vtkIdType HeaderLines;
  vtkIdType ColumnNamesOnLine;
  vtkIdType SkipColumnNames;

  bool RecordAdjacent;
  bool MergeConsDelims;
  bool ProcessEscapeSequence;
  bool UseStringDelimiter;
  vtkTypeUInt32 WithinString;
};

} // End anonymous namespace

/////////////////////////////////////////////////////////////////////////////////////////
// vtkTecplotTableReader

vtkStandardNewMacro(vtkTecplotTableReader);

vtkTecplotTableReader::vtkTecplotTableReader()
  : FileName(nullptr)
  , MaxRecords(0)
  , HeaderLines(2)
  , ColumnNamesOnLine(1)
  , SkipColumnNames(1)
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
  this->PedigreeIdArrayName = nullptr;
  this->SetPedigreeIdArrayName("id");
  this->GeneratePedigreeIds = false;
  this->OutputPedigreeIds = false;
}

vtkTecplotTableReader::~vtkTecplotTableReader()
{
  this->SetPedigreeIdArrayName(nullptr);
  this->SetFileName(nullptr);
}

void vtkTecplotTableReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << (this->FileName ? this->FileName : "(none)") << endl;
  os << indent << "MaxRecords: " << this->MaxRecords << endl;
  os << indent << "GeneratePedigreeIds: " << this->GeneratePedigreeIds << endl;
  os << indent << "PedigreeIdArrayName: " << this->PedigreeIdArrayName << endl;
  os << indent << "OutputPedigreeIds: " << (this->OutputPedigreeIds ? "true" : "false") << endl;
}

vtkStdString vtkTecplotTableReader::GetLastError()
{
  return this->LastError;
}

int vtkTecplotTableReader::RequestData(
  vtkInformation*, vtkInformationVector**, vtkInformationVector* outputVector)
{
  vtkTable* const output_table = vtkTable::GetData(outputVector);

  this->LastError = "";

  try
  {
    // We only retrieve one piece ...
    vtkInformation* const outInfo = outputVector->GetInformationObject(0);
    if (outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()) &&
      outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()) > 0)
    {
      return 1;
    }

    if (!this->PedigreeIdArrayName)
    {
      vtkErrorMacro(<< "You must specify a pedigree id array name");
      return 0;
    }

    vtksys::ifstream file_stream;

    // If the filename hasn't been specified, we're done ...
    if (!this->FileName)
    {
      return 1;
    }
    // Get the total size of the input file in bytes
    file_stream.open(this->FileName, ios::binary);
    if (!file_stream.good())
    {
      vtkErrorMacro(<< "Unable to open input file" << std::string(this->FileName));
      return 0;
    }

    file_stream.seekg(0, ios::end);
    file_stream.seekg(0, ios::beg);

    vtkTextCodec* transCodec = vtkTextCodecFactory::CodecToHandle(file_stream);

    if (nullptr == transCodec)
    {
      // should this use the locale instead??
      return 1;
    }

    DelimitedTextIterator iterator(output_table, this->MaxRecords, this->HeaderLines,
      this->ColumnNamesOnLine, this->SkipColumnNames);

    transCodec->ToUnicode(file_stream, iterator);
    iterator.ReachedEndOfInput();
    transCodec->Delete();

    if (this->OutputPedigreeIds)
    {
      vtkAbstractArray* arr = output_table->GetColumnByName(this->PedigreeIdArrayName);

      if (this->GeneratePedigreeIds || !arr)
      {
        vtkSmartPointer<vtkIdTypeArray> pedigreeIds = vtkSmartPointer<vtkIdTypeArray>::New();
        vtkIdType numRows = output_table->GetNumberOfRows();
        pedigreeIds->SetNumberOfTuples(numRows);
        pedigreeIds->SetName(this->PedigreeIdArrayName);
        for (vtkIdType i = 0; i < numRows; ++i)
        {
          pedigreeIds->InsertValue(i, i);
        }
        output_table->GetRowData()->SetPedigreeIds(pedigreeIds);
      }
      else
      {
        if (arr)
        {
          output_table->GetRowData()->SetPedigreeIds(arr);
        }
        else
        {
          vtkErrorMacro(<< "Could not find pedigree id array: "
                        << std::string(this->PedigreeIdArrayName));
          return 0;
        }
      }
    }
  }
  catch (std::exception& e)
  {
    vtkErrorMacro(<< "caught exception: " << e.what());
    this->LastError = e.what();
    output_table->Initialize();
    return 0;
  }
  catch (...)
  {
    vtkErrorMacro(<< "caught unknown exception.");
    this->LastError = "Unknown exception.";
    output_table->Initialize();
    return 0;
  }

  return 1;
}
VTK_ABI_NAMESPACE_END
