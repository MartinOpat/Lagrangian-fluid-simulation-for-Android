// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
#include "vtkArrayDataWriter.h"

#include "vtkArrayData.h"
#include "vtkArrayWriter.h"
#include "vtkExecutive.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtksys/FStream.hxx"

#include <sstream>
#include <stdexcept>

VTK_ABI_NAMESPACE_BEGIN
vtkStandardNewMacro(vtkArrayDataWriter);

vtkArrayDataWriter::vtkArrayDataWriter()
  : FileName(nullptr)
  , Binary(false)
  , WriteToOutputString(false)
{
}

vtkArrayDataWriter::~vtkArrayDataWriter()
{
  this->SetFileName(nullptr);
}

void vtkArrayDataWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << (this->FileName ? this->FileName : "(none)") << endl;
  os << indent << "Binary: " << this->Binary << endl;
  os << indent << "WriteToOutputString: " << (this->WriteToOutputString ? "on" : "off") << endl;
  os << indent << "OutputString: " << this->OutputString << endl;
}

int vtkArrayDataWriter::FillInputPortInformation(int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkArrayData");
  return 1;
}

void vtkArrayDataWriter::WriteData()
{
  if (this->WriteToOutputString)
  {
    this->OutputString = this->Write(this->Binary > 0);
  }
  else
  {
    this->Write(this->FileName ? this->FileName : "", this->Binary > 0);
  }
}

int vtkArrayDataWriter::Write()
{
  return Superclass::Write();
}

bool vtkArrayDataWriter::Write(const vtkStdString& file_name, bool WriteBinary)
{
  vtksys::ofstream file(file_name.c_str(), std::ios::binary);
  return this->Write(file, WriteBinary);
}

bool vtkArrayDataWriter::Write(vtkArrayData* array, const vtkStdString& file_name, bool WriteBinary)
{
  vtksys::ofstream file(file_name.c_str(), std::ios::binary);
  return vtkArrayDataWriter::Write(array, file, WriteBinary);
}

bool vtkArrayDataWriter::Write(ostream& stream, bool WriteBinary)
{
  try
  {
    if (this->GetNumberOfInputConnections(0) != 1)
      throw std::runtime_error("Exactly one input required.");

    vtkArrayData* const array_data =
      vtkArrayData::SafeDownCast(this->GetExecutive()->GetInputData(0, 0));
    if (!array_data)
      throw std::runtime_error("vtkArrayData input required.");

    vtkArrayDataWriter::Write(array_data, stream, WriteBinary);
    return true;
  }
  catch (std::exception& e)
  {
    vtkErrorMacro("caught exception: " << e.what());
  }
  return false;
}

bool vtkArrayDataWriter::Write(vtkArrayData* array_data, ostream& stream, bool WriteBinary)
{
  try
  {
    stream << "vtkArrayData " << array_data->GetNumberOfArrays() << std::endl;
    for (vtkIdType i = 0; i < array_data->GetNumberOfArrays(); ++i)
    {
      vtkArray* const array = array_data->GetArray(i);
      if (!array)
        throw std::runtime_error("Cannot serialize nullptr vtkArray.");

      vtkArrayWriter::Write(array, stream, WriteBinary);
    }
    return true;
  }
  catch (std::exception& e)
  {
    vtkGenericWarningMacro("caught exception: " << e.what());
  }
  return false;
}

vtkStdString vtkArrayDataWriter::Write(bool WriteBinary)
{
  std::ostringstream oss;
  this->Write(oss, WriteBinary);
  return oss.str();
}

vtkStdString vtkArrayDataWriter::Write(vtkArrayData* array_data, bool WriteBinary)
{
  std::ostringstream oss;
  vtkArrayDataWriter::Write(array_data, oss, WriteBinary);
  return oss.str();
}
VTK_ABI_NAMESPACE_END
