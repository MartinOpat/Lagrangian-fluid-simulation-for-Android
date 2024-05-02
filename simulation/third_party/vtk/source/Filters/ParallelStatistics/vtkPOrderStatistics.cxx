// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-FileCopyrightText: Copyright 2011 Sandia Corporation
// SPDX-License-Identifier: LicenseRef-BSD-3-Clause-Sandia-USGov

#include "vtkPOrderStatistics.h"

#include "vtkCommunicator.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkMultiProcessController.h"
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include "vtkTable.h"
#include "vtkVariantArray.h"

#include <map>
#include <set>
#include <vector>

VTK_ABI_NAMESPACE_BEGIN
vtkStandardNewMacro(vtkPOrderStatistics);
vtkCxxSetObjectMacro(vtkPOrderStatistics, Controller, vtkMultiProcessController);
//------------------------------------------------------------------------------
vtkPOrderStatistics::vtkPOrderStatistics()
{
  this->Controller = nullptr;
  this->SetController(vtkMultiProcessController::GetGlobalController());
}

//------------------------------------------------------------------------------
vtkPOrderStatistics::~vtkPOrderStatistics()
{
  this->SetController(nullptr);
}

//------------------------------------------------------------------------------
void vtkPOrderStatistics::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Controller: " << this->Controller << endl;
}

//------------------------------------------------------------------------------
static void StringVectorToStringBuffer(const std::vector<std::string>& strings, std::string& buffer)
{
  buffer.clear();

  for (std::vector<std::string>::const_iterator it = strings.begin(); it != strings.end(); ++it)
  {
    buffer.append(*it);
    buffer.push_back(0);
  }
}

//------------------------------------------------------------------------------
static void StringArrayToStringBuffer(vtkStringArray* sVals, std::string& sPack)
{
  std::vector<std::string> sVect; // consecutive strings

  vtkIdType nv = sVals->GetNumberOfValues();
  for (vtkIdType i = 0; i < nv; ++i)
  {
    // Push back current string value
    sVect.push_back(sVals->GetValue(i));
  }

  // Concatenate vector of strings into single string
  StringVectorToStringBuffer(sVect, sPack);
}

//------------------------------------------------------------------------------
static void StringHistoToBuffers(
  const std::map<vtkStdString, vtkIdType>& histo, std::string& buffer, vtkIdTypeArray* card)
{
  buffer.clear();

  card->SetNumberOfTuples(static_cast<vtkIdType>(histo.size()));

  vtkIdType r = 0;
  for (std::map<vtkStdString, vtkIdType>::const_iterator it = histo.begin(); it != histo.end();
       ++it, ++r)
  {
    buffer.append(it->first);
    card->SetValue(r, it->second);
    buffer.push_back(0);
  }
}

//------------------------------------------------------------------------------
static void StringBufferToStringVector(const std::string& buffer, std::vector<std::string>& strings)
{
  strings.clear();

  const char* const bufferEnd = buffer.data() + buffer.size();

  for (const char* start = buffer.data(); start != bufferEnd; ++start)
  {
    for (const char* finish = start; finish != bufferEnd; ++finish)
    {
      if (!*finish)
      {
        strings.emplace_back(start);
        start = finish;
        break;
      }
    }
  }
}

//------------------------------------------------------------------------------
void vtkPOrderStatistics::Learn(
  vtkTable* inData, vtkTable* inParameters, vtkMultiBlockDataSet* outMeta)
{
  if (!outMeta)
  {
    return;
  }

  // First calculate order statistics on local data set
  this->Superclass::Learn(inData, inParameters, outMeta);

  if (!outMeta || outMeta->GetNumberOfBlocks() < 1)
  {
    // No statistics were calculated.
    return;
  }

  // Make sure that parallel updates are needed, otherwise leave it at that.
  int np = this->Controller->GetNumberOfProcesses();
  if (np < 2)
  {
    return;
  }

  // Get ready for parallel calculations
  vtkCommunicator* com = this->Controller->GetCommunicator();
  if (!com)
  {
    vtkErrorMacro("No parallel communicator.");
  }

  // Figure local process id
  vtkIdType myRank = com->GetLocalProcessId();

  // NB: Use process 0 as sole reducer for now
  vtkIdType rProc = 0;

  // Iterate over primary tables
  unsigned int nBlocks = outMeta->GetNumberOfBlocks();
  for (unsigned int b = 0; b < nBlocks; ++b)
  {
    // Fetch histogram table
    vtkTable* histoTab = vtkTable::SafeDownCast(outMeta->GetBlock(b));
    if (!histoTab)
    {
      continue;
    }

    // Downcast columns to typed arrays for efficient data access
    vtkAbstractArray* vals = histoTab->GetColumnByName("Value");
    vtkIdTypeArray* card =
      vtkArrayDownCast<vtkIdTypeArray>(histoTab->GetColumnByName("Cardinality"));
    if (!vals || !card)
    {
      vtkErrorMacro("Column fetching error on process " << myRank << ".");

      return;
    }

    // Create new table for global histogram
    vtkTable* histoTab_g = vtkTable::New();

    // Create column for global histogram cardinalities
    vtkIdTypeArray* card_g = vtkIdTypeArray::New();
    card_g->SetName("Cardinality");

    // Gather all histogram cardinalities on process rProc
    // NB: GatherV because the arrays have variable lengths
    if (!com->GatherV(card, card_g, rProc))
    {
      vtkErrorMacro(
        "Process " << com->GetLocalProcessId() << " could not gather histogram cardinalities.");

      return;
    }

    // Gather all histogram values on rProc and perform reduction of the global histogram table
    if (vals->IsA("vtkDataArray"))
    {
      // Downcast column to data array for subsequent typed message passing
      vtkDataArray* dVals = vtkArrayDownCast<vtkDataArray>(vals);

      // Create column for global histogram values of the same type as the values
      vtkDataArray* dVals_g = vtkDataArray::CreateDataArray(dVals->GetDataType());
      dVals_g->SetName("Value");

      // Gather all histogram values on process rProc
      // NB: GatherV because the arrays have variable lengths
      if (!com->GatherV(dVals, dVals_g, rProc))
      {
        vtkErrorMacro(
          "Process " << com->GetLocalProcessId() << " could not gather histogram values.");

        return;
      }

      // Reduce to global histogram table on process rProc
      if (myRank == rProc)
      {
        if (this->Reduce(card_g, dVals_g))
        {
          return;
        }
      } // if ( myRank == rProc )

      // Finally broadcast reduced histogram values
      if (!com->Broadcast(dVals_g, rProc))
      {
        vtkErrorMacro("Process " << com->GetLocalProcessId()
                                 << " could not broadcast reduced histogram values.");

        return;
      }

      // Add column of data values to histogram table
      histoTab_g->AddColumn(dVals_g);

      // Clean up
      dVals_g->Delete();

      // Finally broadcast reduced histogram cardinalities
      if (!com->Broadcast(card_g, rProc))
      {
        vtkErrorMacro("Process " << com->GetLocalProcessId()
                                 << " could not broadcast reduced histogram cardinalities.");

        return;
      }
    } // if ( vals->IsA("vtkDataArray") )
    else if (vals->IsA("vtkStringArray"))
    {
      // Downcast column to string array for subsequent typed message passing
      vtkStringArray* sVals = vtkArrayDownCast<vtkStringArray>(vals);

      // Packing step: concatenate all string values
      std::string sPack_l;
      StringArrayToStringBuffer(sVals, sPack_l);

      // (All) gather all string sizes
      vtkIdType nc_l = static_cast<vtkIdType>(sPack_l.size());
      vtkIdType* nc_g = new vtkIdType[np];
      com->AllGather(&nc_l, nc_g, 1);

      // Calculate total size and displacement arrays
      vtkIdType* offsets = new vtkIdType[np];
      vtkIdType ncTotal = 0;

      for (vtkIdType i = 0; i < np; ++i)
      {
        offsets[i] = ncTotal;
        ncTotal += nc_g[i];
      }

      // Allocate receive buffer on reducer process, based on the global size obtained above
      char* sPack_g = nullptr;
      if (myRank == rProc)
      {
        sPack_g = new char[ncTotal];
      }

      // Gather all sPack on process rProc
      // NB: GatherV because the packets have variable lengths
      if (!com->GatherV(&(*sPack_l.begin()), sPack_g, nc_l, nc_g, offsets, rProc))
      {
        vtkErrorMacro("Process " << myRank << "could not gather string values.");
        delete[] nc_g;
        delete[] offsets;
        delete[] sPack_g;

        return;
      }
      delete[] nc_g;
      delete[] offsets;

      // Reduce to global histogram on process rProc
      std::map<vtkStdString, vtkIdType> histogram;
      if (myRank == rProc)
      {
        if (this->Reduce(card_g, ncTotal, sPack_g, histogram))
        {
          delete[] sPack_g;
          return;
        }
      } // if ( myRank == rProc )
      delete[] sPack_g;

      // Create column for global histogram values of the same type as the values
      vtkStringArray* sVals_g = vtkStringArray::New();
      sVals_g->SetName("Value");

      // Finally broadcast reduced histogram
      if (this->Broadcast(histogram, card_g, sVals_g, rProc))
      {
        vtkErrorMacro("Process " << com->GetLocalProcessId()
                                 << " could not broadcast reduced histogram values.");
        sVals_g->Delete();

        return;
      }

      // Add column of string values to histogram table
      histoTab_g->AddColumn(sVals_g);

      // Clean up
      sVals_g->Delete();
    } // else if ( vals->IsA("vtkStringArray") )
    else if (vals->IsA("vtkVariantArray"))
    {
      vtkErrorMacro(
        "Unsupported data type (variant array) for column " << vals->GetName() << ". Ignoring it.");
      return;
    }
    else
    {
      vtkErrorMacro("Unsupported data type for column " << vals->GetName() << ". Ignoring it.");
      return;
    }

    // Add column of cardinalities to histogram table
    histoTab_g->AddColumn(card_g);

    // Replace local histogram table with globally reduced one
    outMeta->SetBlock(b, histoTab_g);

    // Clean up
    card_g->Delete();
    histoTab_g->Delete();
  } // for ( unsigned int b = 0; b < nBlocks; ++ b )
}

//------------------------------------------------------------------------------
bool vtkPOrderStatistics::Reduce(vtkIdTypeArray* card_g, vtkDataArray* dVals_g)
{
  // Check consistency: we must have as many values as cardinality entries
  vtkIdType nRow_g = card_g->GetNumberOfTuples();
  if (dVals_g->GetNumberOfTuples() != nRow_g)
  {
    vtkErrorMacro("Gathering error on process "
      << this->Controller->GetCommunicator()->GetLocalProcessId()
      << ": inconsistent number of values and cardinality entries: " << dVals_g->GetNumberOfTuples()
      << " <> " << nRow_g << ".");

    return true;
  }

  // Reduce to the global histogram table
  std::map<double, vtkIdType> histogram;
  double x;
  vtkIdType c;
  for (vtkIdType r = 0; r < nRow_g; ++r)
  {
    // First, fetch value
    x = dVals_g->GetTuple1(r);

    // Then, retrieve corresponding cardinality
    c = card_g->GetValue(r);

    // Last, update histogram count for corresponding value
    histogram[x] += c;
  }

  // Now resize global histogram arrays to reduced size
  nRow_g = static_cast<vtkIdType>(histogram.size());
  dVals_g->SetNumberOfTuples(nRow_g);
  card_g->SetNumberOfTuples(nRow_g);

  // Then store reduced histogram into array
  std::map<double, vtkIdType>::iterator hit = histogram.begin();
  for (vtkIdType r = 0; r < nRow_g; ++r, ++hit)
  {
    dVals_g->SetTuple1(r, hit->first);
    card_g->SetValue(r, hit->second);
  }

  return false;
}

//------------------------------------------------------------------------------
bool vtkPOrderStatistics::Reduce(vtkIdTypeArray* card_g, vtkIdType& ncTotal, char* sPack_g,
  std::map<vtkStdString, vtkIdType>& histogram)
{
  // First, unpack the packet of strings
  std::vector<std::string> sVect_g;
  StringBufferToStringVector(std::string(sPack_g, ncTotal), sVect_g);

  // Second, check consistency: we must have as many values as cardinality entries
  vtkIdType nRow_g = card_g->GetNumberOfTuples();
  if (vtkIdType(sVect_g.size()) != nRow_g)
  {
    vtkErrorMacro("Gathering error on process "
      << this->Controller->GetCommunicator()->GetLocalProcessId()
      << ": inconsistent number of values and cardinality entries: " << sVect_g.size() << " <> "
      << nRow_g << ".");

    return true;
  }

  // Third, reduce to the global histogram
  vtkIdType c;
  vtkIdType i = 0;
  for (std::vector<std::string>::iterator vit = sVect_g.begin(); vit != sVect_g.end(); ++vit, ++i)
  {
    // First, retrieve cardinality
    c = card_g->GetValue(i);

    // Then, update histogram count for corresponding value
    histogram[*vit] += c;
  }

  return false;
}

//------------------------------------------------------------------------------
bool vtkPOrderStatistics::Broadcast(std::map<vtkStdString, vtkIdType>& histogram,
  vtkIdTypeArray* card, vtkStringArray* sVals, vtkIdType rProc)
{
  vtkCommunicator* com = this->Controller->GetCommunicator();

  // Concatenate string keys of histogram into single string and put values into resized array
  std::string sPack;
  StringHistoToBuffers(histogram, sPack, card);

  // Broadcast size of string buffer
  vtkIdType nc = static_cast<vtkIdType>(sPack.size());
  if (!com->Broadcast(&nc, 1, rProc))
  {
    vtkErrorMacro(
      "Process " << com->GetLocalProcessId() << " could not broadcast size of string buffer.");

    return true;
  }

  // Resize string so it can receive the broadcasted string buffer
  sPack.resize(nc);

  // Broadcast histogram values
  if (!com->Broadcast(&(*sPack.begin()), nc, rProc))
  {
    vtkErrorMacro(
      "Process " << com->GetLocalProcessId() << " could not broadcast histogram string values.");

    return true;
  }

  // Unpack the packet of strings
  std::vector<std::string> sVect;
  StringBufferToStringVector(sPack, sVect);

  // Broadcast histogram cardinalities
  if (!com->Broadcast(card, rProc))
  {
    vtkErrorMacro(
      "Process " << com->GetLocalProcessId() << " could not broadcast histogram cardinalities.");

    return true;
  }

  // Now resize global histogram arrays to reduced size
  sVals->SetNumberOfValues(static_cast<vtkIdType>(sVect.size()));

  // Then store reduced histogram into array
  vtkIdType r = 0;
  for (std::vector<std::string>::iterator vit = sVect.begin(); vit != sVect.end(); ++vit, ++r)
  {
    sVals->SetValue(r, *vit);
  }

  return false;
}
VTK_ABI_NAMESPACE_END
