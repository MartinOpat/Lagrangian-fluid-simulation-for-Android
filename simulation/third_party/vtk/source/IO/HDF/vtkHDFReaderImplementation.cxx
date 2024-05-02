// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause

#include "vtkHDFReaderImplementation.h"
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <type_traits>

#include "vtkAMRBox.h"
#include "vtkAMRUtilities.h"
#include "vtkCharArray.h"
#include "vtkDataArrayRange.h"
#include "vtkDataArraySelection.h"
#include "vtkDataObject.h"
#include "vtkDoubleArray.h"
#include "vtkFieldData.h"
#include "vtkFloatArray.h"
#include "vtkHDF5ScopedHandle.h"
#include "vtkIdTypeArray.h"
#include "vtkIntArray.h"
#include "vtkLogger.h"
#include "vtkLongArray.h"
#include "vtkLongLongArray.h"
#include "vtkOverlappingAMR.h"
#include "vtkShortArray.h"
#include "vtkStringArray.h"
#include "vtkUniformGrid.h"
#include "vtkUnsignedCharArray.h"
#include "vtkUnsignedIntArray.h"
#include "vtkUnsignedLongArray.h"
#include "vtkUnsignedLongLongArray.h"
#include "vtkUnsignedShortArray.h"

#include <array>

//------------------------------------------------------------------------------
VTK_ABI_NAMESPACE_BEGIN
namespace
{

const std::map<int, std::string> ARRAY_OFFSET_GROUPS = { { 0, "PointDataOffsets" },
  { 1, "CellDataOffsets" }, { 2, "FieldDataOffsets" } };

herr_t AddName(hid_t group, const char* name, const H5L_info_t*, void* op_data)
{
  auto array = static_cast<std::vector<std::string>*>(op_data);
  herr_t status = -1;
  H5O_info_t infobuf;
  if ((status = H5Oget_info_by_name(group, name, &infobuf, H5P_DEFAULT)) >= 0 &&
    infobuf.type == H5O_TYPE_DATASET)
  {
    array->push_back(name);
  }
  return status;
}
};

//------------------------------------------------------------------------------
vtkHDFReader::Implementation::TypeDescription vtkHDFReader::Implementation::GetTypeDescription(
  hid_t type)
{
  TypeDescription td;
  td.Class = H5Tget_class(type);
  td.Size = H5Tget_size(type);
  if (td.Class == H5T_INTEGER)
  {
    td.Sign = H5Tget_sign(type);
  }
  return td;
}

//------------------------------------------------------------------------------
vtkHDFReader::Implementation::Implementation(vtkHDFReader* reader)
  : File(-1)
  , VTKGroup(-1)
  , DataSetType(-1)
  , NumberOfPieces(-1)
  , Reader(reader)
{
  std::fill(this->AttributeDataGroup.begin(), this->AttributeDataGroup.end(), -1);
  std::fill(this->Version.begin(), this->Version.end(), 0);
}

//------------------------------------------------------------------------------
vtkHDFReader::Implementation::~Implementation()
{
  this->Close();
}

std::vector<hsize_t> vtkHDFReader::Implementation::GetDimensions(const char* datasetName)
{
  std::vector<hsize_t> dims;

  vtkHDF::ScopedH5DHandle dataset = H5Dopen(this->File, datasetName, H5P_DEFAULT);
  if (dataset < 0)
  {
    vtkErrorWithObjectMacro(this->Reader, << std::string("Cannot open ") + datasetName);
    return dims;
  }

  vtkHDF::ScopedH5SHandle dataspace = H5Dget_space(dataset);
  if (dataspace < 0)
  {
    vtkErrorWithObjectMacro(
      this->Reader, << std::string("Cannot get space for dataset ") + datasetName);
    return dims;
  }

  int rank = H5Sget_simple_extent_ndims(dataspace);
  if (rank < 0)
  {
    vtkErrorWithObjectMacro(
      this->Reader, << std::string(datasetName) + " dataset: get_simple_extent_ndims error");
    return dims;
  }

  if (rank > 0)
  {
    dims.resize(rank, 0);
    if (H5Sget_simple_extent_dims(dataspace, dims.data(), nullptr) < 0)
    {
      vtkErrorWithObjectMacro(
        this->Reader, << std::string("Cannot find dimension for ") + datasetName);
      dims.clear();
      return dims;
    }
  }

  return dims;
}

//------------------------------------------------------------------------------
bool vtkHDFReader::Implementation::Open(const char* fileName)
{
  bool error = false;
  if (!fileName)
  {
    vtkErrorWithObjectMacro(this->Reader, "Invalid filename: " << fileName);
    return false;
  }
  if (this->FileName.empty() || this->FileName != fileName)
  {
    this->FileName = fileName;
    if (this->File >= 0)
    {
      this->Close();
    }
    // turn off error logging and save error function
    H5E_auto_t f;
    void* client_data;
    H5Eget_auto(H5E_DEFAULT, &f, &client_data);
    H5Eset_auto(H5E_DEFAULT, nullptr, nullptr);
    if ((this->File = H5Fopen(this->FileName.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT)) < 0)
    {
      // we try to read a non-HDF file
      return false;
    }
    if ((this->VTKGroup = H5Gopen(this->File, "/VTKHDF", H5P_DEFAULT)) < 0)
    {
      // we try to read a non-VTKHDF file
      return false;
    }

    H5Eset_auto(H5E_DEFAULT, f, client_data);
    if (!this->ReadDataSetType())
    {
      return false;
    }
    H5Eset_auto(H5E_DEFAULT, nullptr, nullptr);

    std::array<const char*, 3> groupNames = { "/VTKHDF/PointData", "/VTKHDF/CellData",
      "/VTKHDF/FieldData" };

    if (this->DataSetType == VTK_OVERLAPPING_AMR)
    {
      groupNames = { "/VTKHDF/Level0/PointData", "/VTKHDF/Level0/CellData",
        "/VTKHDF/Level0/FieldData" };
    }

    // try to open cell or point group. Its OK if they don't exist.
    for (size_t i = 0; i < this->AttributeDataGroup.size(); ++i)
    {
      this->AttributeDataGroup[i] = H5Gopen(this->File, groupNames[i], H5P_DEFAULT);
    }
    // turn on error logging and restore error function
    H5Eset_auto(H5E_DEFAULT, f, client_data);
    if (!GetAttribute("Version", this->Version.size(), this->Version.data()))
    {
      return false;
    }

    H5Eset_auto(H5E_DEFAULT, nullptr, nullptr);
    // get transient information if there is any
    vtkIdType nSteps = this->GetNumberOfSteps();
    H5Eset_auto(H5E_DEFAULT, f, client_data);

    try
    {
      if (this->DataSetType == VTK_UNSTRUCTURED_GRID || this->DataSetType == VTK_POLY_DATA)
      {
        const char* datasetName = "/VTKHDF/NumberOfPoints";
        std::vector<hsize_t> dims = this->GetDimensions(datasetName);
        if (dims.size() != 1)
        {
          throw std::runtime_error(std::string(datasetName) + " dataset should have 1 dimension");
        }
        // Case where the data set has the same number of pieces for  all steps in the dataset
        this->NumberOfPieces = dims[0] / nSteps;
      }
      else if (this->DataSetType == VTK_IMAGE_DATA || this->DataSetType == VTK_OVERLAPPING_AMR)
      {
        this->NumberOfPieces = 1;
      }
    }
    catch (const std::exception& e)
    {
      vtkErrorWithObjectMacro(this->Reader, << e.what());
      error = true;
    }
  }
  this->BuildTypeReaderMap();
  return !error;
}

//------------------------------------------------------------------------------
bool vtkHDFReader::Implementation::ReadDataSetType()
{
  if (H5Aexists(this->VTKGroup, "Type"))
  {
    vtkHDF::ScopedH5AHandle typeAttributeHID = H5Aopen_name(this->VTKGroup, "Type");
    if (typeAttributeHID < 0)
    {
      vtkErrorWithObjectMacro(this->Reader, "Can't open 'Type' attribute.");
      return false;
    }

    vtkHDF::ScopedH5THandle hdfType = H5Aget_type(typeAttributeHID);
    if (hdfType == H5I_INVALID_HID)
    {
      vtkErrorWithObjectMacro(this->Reader, "Invalid type when reading type attribute.");
      return false;
    }

    H5T_class_t attributeClass = H5Tget_class(hdfType);
    if (attributeClass != H5T_STRING)
    {
      vtkErrorWithObjectMacro(this->Reader, "Can't get class type of attribute.");
      return false;
    }

    H5T_cset_t characterType = H5Tget_cset(hdfType);
    if (characterType != H5T_CSET_ASCII)
    {
      vtkErrorWithObjectMacro(
        this->Reader, "Not an ASCII string character type: " << characterType);
      return false;
    }

    hsize_t stringLength = H5Aget_storage_size(typeAttributeHID);
    if (stringLength < 1 || stringLength > 32)
    {
      vtkErrorWithObjectMacro(this->Reader,
        "Wrong length of Type attribute (expected between 1 and 32): " << stringLength);
      return false;
    }
    std::array<char, 32> stringArray;
    if (H5Aread(typeAttributeHID, hdfType, stringArray.data()) < 0)
    {
      vtkErrorWithObjectMacro(
        this->Reader, "Not an ASCII string character type: " << characterType);
      return false;
    }
    std::string typeName(stringArray.data(), stringLength);

    if (typeName == "OverlappingAMR")
    {
      this->DataSetType = VTK_OVERLAPPING_AMR;
    }
    else if (typeName == "ImageData")
    {
      this->DataSetType = VTK_IMAGE_DATA;
    }
    else if (typeName == "UnstructuredGrid")
    {
      this->DataSetType = VTK_UNSTRUCTURED_GRID;
    }
    else if (typeName == "PolyData")
    {
      this->DataSetType = VTK_POLY_DATA;
    }
    else
    {
      vtkErrorWithObjectMacro(this->Reader, "Unknown data set type: " << typeName);
      return false;
    }
  }
  else
  {
    vtkErrorWithObjectMacro(this->Reader, "Can't find the `Type` attribute.");
    return false;
  }
  return true;
}

//------------------------------------------------------------------------------
std::size_t vtkHDFReader::Implementation::GetNumberOfSteps()
{
  if (this->File < 0)
  {
    vtkErrorWithObjectMacro(this->Reader, "Cannot get number of steps if the file is not open");
  }
  return this->GetNumberOfSteps(this->VTKGroup);
}

//------------------------------------------------------------------------------
std::size_t vtkHDFReader::Implementation::GetNumberOfSteps(hid_t vtkHDFGroup)
{
  if (vtkHDFGroup < 0)
  {
    vtkErrorWithObjectMacro(this->Reader, "Cannot get number of steps if the group is not open");
  }

  if (H5Lexists(vtkHDFGroup, "Steps", H5P_DEFAULT) <= 0)
  {
    // Steps group does not exist and so there is only 1 step
    return 1;
  }

  // Steps group does exist
  vtkHDF::ScopedH5GHandle steps = H5Gopen(vtkHDFGroup, "Steps", H5P_DEFAULT);
  if (steps < 0)
  {
    vtkErrorWithObjectMacro(this->Reader, "Could not open steps group");
    return 1;
  }

  int nSteps = 1;
  this->GetAttribute(steps, "NSteps", 1, &nSteps);
  return nSteps > 0 ? static_cast<std::size_t>(nSteps) : 1;
}

//------------------------------------------------------------------------------
int vtkHDFReader::Implementation::GetNumberOfPieces(vtkIdType step)
{
  if (step < 0 || this->GetNumberOfSteps() == 1 ||
    H5Lexists(this->VTKGroup, "Steps/NumberOfParts", H5P_DEFAULT) <= 0)
  {
    return this->NumberOfPieces;
  }
  std::vector<vtkIdType> buffer = this->GetMetadata("Steps/NumberOfParts", 1, step);
  if (buffer.empty())
  {
    vtkErrorWithObjectMacro(
      nullptr, "Could not read step " << step << " in NumberOfParts data set.");
    return -1;
  }
  this->NumberOfPieces = buffer[0];
  return this->NumberOfPieces;
}

//------------------------------------------------------------------------------
void vtkHDFReader::Implementation::Close()
{
  this->DataSetType = -1;
  this->NumberOfPieces = 0;
  std::fill(this->Version.begin(), this->Version.end(), 0);
  for (size_t i = 0; i < this->AttributeDataGroup.size(); ++i)
  {
    if (this->AttributeDataGroup[i] >= 0)
    {
      H5Gclose(this->AttributeDataGroup[i]);
      this->AttributeDataGroup[i] = -1;
    }
  }
  if (this->VTKGroup >= 0)
  {
    H5Gclose(this->VTKGroup);
    this->VTKGroup = -1;
  }
  if (this->File >= 0)
  {
    H5Fclose(this->File);
    this->File = -1;
  }
}

//------------------------------------------------------------------------------
void vtkHDFReader::Implementation::BuildTypeReaderMap()
{
  this->TypeReaderMap[this->GetTypeDescription(H5T_NATIVE_CHAR)] =
    &vtkHDFReader::Implementation::NewArray<char>;
  this->TypeReaderMap[this->GetTypeDescription(H5T_NATIVE_UCHAR)] =
    &vtkHDFReader::Implementation::NewArray<unsigned char>;
  this->TypeReaderMap[this->GetTypeDescription(H5T_NATIVE_SHORT)] =
    &vtkHDFReader::Implementation::NewArray<short>;
  this->TypeReaderMap[this->GetTypeDescription(H5T_NATIVE_USHORT)] =
    &vtkHDFReader::Implementation::NewArray<unsigned short>;
  this->TypeReaderMap[this->GetTypeDescription(H5T_NATIVE_INT)] =
    &vtkHDFReader::Implementation::NewArray<int>;
  this->TypeReaderMap[this->GetTypeDescription(H5T_NATIVE_UINT)] =
    &vtkHDFReader::Implementation::NewArray<unsigned int>;
  if (!this->TypeReaderMap[this->GetTypeDescription(H5T_NATIVE_LONG)])
  {
    // long may be the same as int
    this->TypeReaderMap[this->GetTypeDescription(H5T_NATIVE_LONG)] =
      &vtkHDFReader::Implementation::NewArray<long>;
    this->TypeReaderMap[this->GetTypeDescription(H5T_NATIVE_ULONG)] =
      &vtkHDFReader::Implementation::NewArray<unsigned long>;
  }
  if (!this->TypeReaderMap[this->GetTypeDescription(H5T_NATIVE_LLONG)])
  {
    // long long may be the same as long
    this->TypeReaderMap[this->GetTypeDescription(H5T_NATIVE_LLONG)] =
      &vtkHDFReader::Implementation::NewArray<long long>;
    this->TypeReaderMap[this->GetTypeDescription(H5T_NATIVE_ULLONG)] =
      &vtkHDFReader::Implementation::NewArray<unsigned long long>;
  }
  this->TypeReaderMap[this->GetTypeDescription(H5T_NATIVE_FLOAT)] =
    &vtkHDFReader::Implementation::NewArray<float>;
  this->TypeReaderMap[this->GetTypeDescription(H5T_NATIVE_DOUBLE)] =
    &vtkHDFReader::Implementation::NewArray<double>;
}

//------------------------------------------------------------------------------
template <typename T>
hid_t vtkHDFReader::Implementation::TemplateTypeToHdfNativeType()
{
  if (std::is_same<T, char>::value)
  {
    return H5T_NATIVE_CHAR;
  }
  else if (std::is_same<T, unsigned char>::value)
  {
    return H5T_NATIVE_UCHAR;
  }
  else if (std::is_same<T, short>::value)
  {
    return H5T_NATIVE_SHORT;
  }
  else if (std::is_same<T, unsigned short>::value)
  {
    return H5T_NATIVE_USHORT;
  }
  else if (std::is_same<T, int>::value)
  {
    return H5T_NATIVE_INT;
  }
  else if (std::is_same<T, unsigned int>::value)
  {
    return H5T_NATIVE_UINT;
  }
  else if (std::is_same<T, long>::value)
  {
    return H5T_NATIVE_LONG;
  }
  else if (std::is_same<T, unsigned long>::value)
  {
    return H5T_NATIVE_ULONG;
  }
  else if (std::is_same<T, long long>::value)
  {
    return H5T_NATIVE_LLONG;
  }
  else if (std::is_same<T, unsigned long long>::value)
  {
    return H5T_NATIVE_ULLONG;
  }
  else if (std::is_same<T, float>::value)
  {
    return H5T_NATIVE_FLOAT;
  }
  else if (std::is_same<T, double>::value)
  {
    return H5T_NATIVE_DOUBLE;
  }
  else
  {
    vtkErrorWithObjectMacro(this->Reader, "Invalid type: " << typeid(T).name());
    return -1;
  }
}

//------------------------------------------------------------------------------
template <typename T>
vtkDataArray* vtkHDFReader::Implementation::NewVtkDataArray()
{
  if (std::is_same<T, char>::value)
  {
    return vtkCharArray::New();
  }
  else if (std::is_same<T, unsigned char>::value)
  {
    return vtkUnsignedCharArray::New();
  }
  else if (std::is_same<T, short>::value)
  {
    return vtkShortArray::New();
  }
  else if (std::is_same<T, unsigned short>::value)
  {
    return vtkUnsignedShortArray::New();
  }
  else if (std::is_same<T, int>::value)
  {
    return vtkIntArray::New();
  }
  else if (std::is_same<T, unsigned int>::value)
  {
    return vtkUnsignedIntArray::New();
  }
  else if (std::is_same<T, long>::value)
  {
    return vtkLongArray::New();
  }
  else if (std::is_same<T, unsigned long>::value)
  {
    return vtkUnsignedLongArray::New();
  }
  else if (std::is_same<T, long long>::value)
  {
    return vtkLongLongArray::New();
  }
  else if (std::is_same<T, unsigned long long>::value)
  {
    return vtkUnsignedLongLongArray::New();
  }
  else if (std::is_same<T, float>::value)
  {
    return vtkFloatArray::New();
  }
  else if (std::is_same<T, double>::value)
  {
    return vtkDoubleArray::New();
  }
  else
  {
    vtkErrorWithObjectMacro(this->Reader, "Invalid type: " << typeid(T).name());
    return nullptr;
  }
}

//------------------------------------------------------------------------------
bool vtkHDFReader::Implementation::GetPartitionExtent(hsize_t partitionIndex, int* extent)
{
  const int RANK = 2;
  const char* datasetName = "/VTKHDF/Extents";

  // create the memory space
  hsize_t dimsm[RANK];
  dimsm[0] = 1;
  dimsm[1] = 6;
  vtkHDF::ScopedH5SHandle memspace = H5Screate_simple(RANK, dimsm, nullptr);
  if (memspace < 0)
  {
    vtkErrorWithObjectMacro(this->Reader, << "Error H5Screate_simple for memory space");
    return false;
  }

  // create the file dataspace + hyperslab
  vtkHDF::ScopedH5DHandle dataset = H5Dopen(this->File, datasetName, H5P_DEFAULT);
  if (dataset < 0)
  {
    vtkErrorWithObjectMacro(this->Reader, << std::string("Cannot open ") + datasetName);
    return false;
  }

  hsize_t start[RANK] = { partitionIndex, 0 }, count[RANK] = { 1, 2 };
  vtkHDF::ScopedH5SHandle dataspace = H5Dget_space(dataset);
  if (dataspace < 0)
  {
    vtkErrorWithObjectMacro(
      this->Reader, << std::string("Cannot get space for dataset ") + datasetName);
    return false;
  }

  if (H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, start, nullptr, count, nullptr) < 0)
  {
    vtkErrorWithObjectMacro(
      this->Reader, << std::string("Error selecting hyperslab for ") + datasetName);
    return false;
  }

  // read hyperslab
  if (H5Dread(dataset, H5T_NATIVE_INT, memspace, dataspace, H5P_DEFAULT, extent) < 0)
  {
    vtkErrorWithObjectMacro(
      this->Reader, << std::string("Error reading hyperslab from ") + datasetName);
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
template <typename T>
bool vtkHDFReader::Implementation::GetAttribute(
  const char* attributeName, size_t numberOfElements, T* value)
{
  return this->GetAttribute(this->VTKGroup, attributeName, numberOfElements, value);
}

//------------------------------------------------------------------------------
template <typename T>
bool vtkHDFReader::Implementation::GetAttribute(
  hid_t group, const char* attributeName, size_t numberOfElements, T* value)
{
  vtkHDF::ScopedH5AHandle attr = H5Aopen_name(group, attributeName);
  if (attr < 0)
  {
    vtkErrorWithObjectMacro(this->Reader, << std::string(attributeName) + " attribute not found");
    return false;
  }

  vtkHDF::ScopedH5SHandle space = H5Aget_space(attr);
  if (space < 0)
  {
    vtkErrorWithObjectMacro(
      this->Reader, << std::string(attributeName) + " attribute: get_space error");
    return false;
  }
  int ndims = H5Sget_simple_extent_ndims(space);
  if (ndims < 0)
  {
    vtkErrorWithObjectMacro(
      this->Reader, << std::string(attributeName) + " attribute: get_simple_extent_ndims error");
    return false;
  }

  if (ndims > 1)
  {
    vtkErrorWithObjectMacro(this->Reader,
      << std::string(attributeName) + " attribute should have rank 1 or 0, it has rank " << ndims);
    return false;
  }

  if (ndims == 0 && numberOfElements != 1)
  {
    vtkErrorWithObjectMacro(this->Reader,
      << std::string(attributeName) + " attribute should have rank 1, it has rank " << ndims);
    return false;
  }

  hsize_t ne = 0;
  if (H5Sget_simple_extent_dims(space, &ne, nullptr) < 0)
  {
    vtkErrorWithObjectMacro(
      this->Reader, << std::string("Cannot find dimension for ") + attributeName);
    return false;
  }

  if (numberOfElements != 1 && ne != numberOfElements)
  {
    vtkErrorWithObjectMacro(this->Reader,
      << attributeName << " attribute should have " << numberOfElements << " dimensions");
    return false;
  }
  hid_t hdfType = this->TemplateTypeToHdfNativeType<T>();
  if (hdfType < 0)
  {
    vtkErrorWithObjectMacro(
      this->Reader, << std::string("Native type not implemented: ") + typeid(T).name());
    return false;
  }

  if (H5Aread(attr, hdfType, value) < 0)
  {
    vtkErrorWithObjectMacro(
      this->Reader, << std::string("Error reading ") + attributeName + " attribute");
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
std::vector<std::string> vtkHDFReader::Implementation::GetArrayNames(int attributeType)
{
  std::vector<std::string> array;
  hid_t group = this->AttributeDataGroup[attributeType];
  if (group > 0)
  {
    // H5_INDEX_CRT_ORDER failed with: no creation order index to query
    H5Literate(group, H5_INDEX_NAME, H5_ITER_NATIVE, nullptr, AddName, &array);
  }
  return array;
}

//------------------------------------------------------------------------------
hid_t vtkHDFReader::Implementation::OpenDataSet(
  hid_t group, const char* name, hid_t* nativeType, std::vector<hsize_t>& dims)
{
  vtkHDF::ScopedH5DHandle dataset = H5Dopen(group, name, H5P_DEFAULT);
  if (dataset < 0)
  {
    vtkErrorWithObjectMacro(this->Reader, << std::string("Cannot open ") + name);
    return -1;
  }

  vtkHDF::ScopedH5THandle datatype = H5Dget_type(dataset);
  if (datatype < 0)
  {
    vtkErrorWithObjectMacro(this->Reader, << std::string("Cannot get_type for dataset: ") + name);
    return -1;
  }

  vtkHDF::ScopedH5SHandle dataspace = H5Dget_space(dataset);
  if (dataspace < 0)
  {
    vtkErrorWithObjectMacro(this->Reader, << std::string("Cannot get space for dataset ") + name);
    return -1;
  }

  if ((*nativeType = H5Tget_native_type(datatype, H5T_DIR_ASCEND)) < 0)
  {
    vtkErrorWithObjectMacro(this->Reader, << std::string("Cannot get type for dataset ") + name);
    return -1;
  }
  int ndims = H5Sget_simple_extent_ndims(dataspace);
  if (ndims < 0)
  {
    vtkErrorWithObjectMacro(
      this->Reader, << std::string(name) + " dataset: get_simple_extent_ndims error");
    return -1;
  }
  dims.resize(ndims);
  if (H5Sget_simple_extent_dims(dataspace, dims.data(), nullptr) < 0)
  {
    vtkErrorWithObjectMacro(this->Reader, << std::string("Cannot find dimension for ") + name);
    return -1;
  }

  hid_t datasetIDToReturn = dataset;
  dataset = -1;
  return datasetIDToReturn;
}

//------------------------------------------------------------------------------
vtkDataArray* vtkHDFReader::Implementation::NewArray(
  int attributeType, const char* name, const std::vector<hsize_t>& fileExtent)
{
  return NewArrayForGroup(this->AttributeDataGroup[attributeType], name, fileExtent);
}

//------------------------------------------------------------------------------
vtkDataArray* vtkHDFReader::Implementation::NewArray(
  int attributeType, const char* name, hsize_t offset, hsize_t size)
{
  std::vector<hsize_t> fileExtent = { offset, offset + size };
  return NewArrayForGroup(this->AttributeDataGroup[attributeType], name, fileExtent);
}

//------------------------------------------------------------------------------
vtkStringArray* vtkHDFReader::Implementation::NewStringArray(hid_t dataset, hsize_t size)
{
  std::vector<char*> rdata(size);

  /*
   * Create the memory datatype.
   */
  hid_t memtype = H5Tcopy(H5T_C_S1);
  if (H5Tset_size(memtype, H5T_VARIABLE) < 0)
  {
    vtkErrorWithObjectMacro(this->Reader, << "Error H5Tset_size");
    return nullptr;
  }

  /*
   * Read the data.
   */
  if (H5Dread(dataset, memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, rdata.data()) < 0)
  {
    vtkErrorWithObjectMacro(this->Reader, << "Error H5Dread");
  }

  auto array = vtkStringArray::New();
  array->SetNumberOfTuples(size);
  for (size_t i = 0; i < size; ++i)
  {
    array->SetValue(i, rdata[i]);
  }

  /*
   * Close and release resources.  Note that H5Dvlen_reclaim works
   * for variable-length strings as well as variable-length arrays.
   * Also note that we must still free the array of pointers stored
   * in rdata, as H5Tvlen_reclaim only frees the data these point to.
   */
  vtkHDF::ScopedH5SHandle space = H5Dget_space(dataset);
  if (H5Dvlen_reclaim(memtype, space, H5P_DEFAULT, rdata.data()) < 0)
  {
    vtkErrorWithObjectMacro(this->Reader, << "Error H5Dvlen_reclaim");
  }

  return array;
}

//------------------------------------------------------------------------------
vtkAbstractArray* vtkHDFReader::Implementation::NewFieldArray(
  const char* name, vtkIdType offset, vtkIdType size)
{
  hid_t tempNativeType = -1;
  std::vector<hsize_t> dims;
  vtkHDF::ScopedH5DHandle dataset =
    this->OpenDataSet(this->AttributeDataGroup[vtkDataObject::FIELD], name, &tempNativeType, dims);
  vtkHDF::ScopedH5THandle nativeType = tempNativeType;
  if (dataset < 0)
  {
    return nullptr;
  }
  TypeDescription td = GetTypeDescription(nativeType);
  if (td.Class == H5T_STRING)
  {
    vtkStringArray* array = nullptr;
    if (dims.size() == 1)
    {
      array = this->NewStringArray(dataset, dims[0]);
    }
    else
    {
      vtkErrorWithObjectMacro(this->Reader, << "Error: String array expected "
                                               "dimensions one but got: "
                                            << dims.size());
    }
    return array;
  }
  else
  {
    // empty fileExtent means read all values from the file
    // field arrays are always 1D
    std::vector<hsize_t> fileExtent;
    if (offset >= 0 || size > 0)
    {
      fileExtent.resize(2, 0);
      fileExtent[0] = offset;
      fileExtent[1] = offset + size;
    }
    return NewArrayForGroup(this->AttributeDataGroup[vtkDataObject::FIELD], name, fileExtent);
  }
}

//------------------------------------------------------------------------------
vtkDataArray* vtkHDFReader::Implementation::NewMetadataArray(
  const char* name, hsize_t offset, hsize_t size)
{
  std::vector<hsize_t> fileExtent = { offset, offset + size };
  return NewArrayForGroup(this->VTKGroup, name, fileExtent);
}

//------------------------------------------------------------------------------
std::vector<vtkIdType> vtkHDFReader::Implementation::GetMetadata(
  const char* name, hsize_t size, hsize_t offset)
{
  std::vector<vtkIdType> v;
  std::vector<hsize_t> fileExtent = { offset, offset + size };
  auto a = vtk::TakeSmartPointer(NewArrayForGroup(this->VTKGroup, name, fileExtent));
  if (!a)
  {
    return v;
  }
  v.resize(a->GetNumberOfTuples() * a->GetNumberOfComponents());
  auto range = vtk::DataArrayValueRange(a);
  std::copy(range.begin(), range.end(), v.begin());
  return v;
}

//------------------------------------------------------------------------------
vtkDataArray* vtkHDFReader::Implementation::NewArrayForGroup(
  hid_t group, const char* name, const std::vector<hsize_t>& parameterExtent)
{
  std::vector<hsize_t> dims;
  hid_t tempNativeType = H5I_INVALID_HID;
  vtkHDF::ScopedH5DHandle dataset = this->OpenDataSet(group, name, &tempNativeType, dims);
  vtkHDF::ScopedH5THandle nativeType = tempNativeType;
  if (dataset < 0)
  {
    return nullptr;
  }

  return this->NewArrayForGroup(dataset, nativeType, dims, parameterExtent);
}

//------------------------------------------------------------------------------
vtkDataArray* vtkHDFReader::Implementation::NewArrayForGroup(hid_t dataset, const hid_t nativeType,
  const std::vector<hsize_t>& dims, const std::vector<hsize_t>& parameterExtent)
{
  vtkDataArray* array = nullptr;
  try
  {
    // used for field arrays
    std::vector<hsize_t> extent = parameterExtent;
    if (extent.empty())
    {
      extent.resize(2, 0);
      extent[1] = dims[0];
      if (dims.size() > 2)
      {
        throw std::runtime_error("Field arrays cannot have more than 2 dimensions.");
      }
    }

    if (dims.size() < (extent.size() >> 1))
    {
      std::ostringstream ostr;
      ostr << "Dataset: Expecting ndims >= " << (extent.size() >> 1) << ", got: " << dims.size();
      throw std::runtime_error(ostr.str());
    }

    hsize_t numberOfComponents = 0;
    if (dims.size() == (extent.size() >> 1))
    {
      numberOfComponents = 1;
    }
    else
    {
      numberOfComponents = dims[dims.size() - 1];
      if (dims.size() > (extent.size() >> 1) + 1)
      {
        std::ostringstream ostr;
        ostr << "Dataset: ndims: " << dims.size()
             << " greater than expected ndims: " << (extent.size() >> 1) << " plus one.";
        throw std::runtime_error(ostr.str());
      }
      if (numberOfComponents == 1)
      {
        extent.resize(dims.size() * 2, 0);
        extent[extent.size() - 1] = numberOfComponents;
      }
    }
    auto it = this->TypeReaderMap.find(this->GetTypeDescription(nativeType));
    if (it == this->TypeReaderMap.end())
    {
      vtkErrorWithObjectMacro(this->Reader, "Unknown native datatype: " << nativeType);
    }
    else
    {
      array = (this->*(it->second))(dataset, extent, numberOfComponents);
    }
  }
  catch (const std::exception& e)
  {
    vtkErrorWithObjectMacro(this->Reader, << e.what());
  }

  return array;
}

//------------------------------------------------------------------------------
template <typename T>
vtkDataArray* vtkHDFReader::Implementation::NewArray(
  hid_t dataset, const std::vector<hsize_t>& fileExtent, hsize_t numberOfComponents)
{
  int numberOfTuples = 1;
  size_t ndims = fileExtent.size() >> 1;
  for (size_t i = 0; i < ndims; ++i)
  {
    size_t j = i << 1;
    numberOfTuples *= (fileExtent[j + 1] - fileExtent[j]);
  }
  auto array = vtkAOSDataArrayTemplate<T>::SafeDownCast(NewVtkDataArray<T>());
  array->SetNumberOfComponents(numberOfComponents);
  array->SetNumberOfTuples(numberOfTuples);
  T* data = array->GetPointer(0);
  if (!this->NewArray(dataset, fileExtent, numberOfComponents, data))
  {
    array->Delete();
    array = nullptr;
  }
  return array;
}

//------------------------------------------------------------------------------
template <typename T>
bool vtkHDFReader::Implementation::NewArray(
  hid_t dataset, const std::vector<hsize_t>& fileExtent, hsize_t numberOfComponents, T* data)
{
  hid_t nativeType = TemplateTypeToHdfNativeType<T>();
  std::vector<hsize_t> count(fileExtent.size() >> 1), start(fileExtent.size() >> 1);
  for (size_t i = 0; i < count.size(); ++i)
  {
    count[i] = fileExtent[i * 2 + 1] - fileExtent[i * 2];
    start[i] = fileExtent[i * 2];
  }
  if (numberOfComponents > 1)
  {
    count.push_back(numberOfComponents);
    start.push_back(0);
  }
  vtkHDF::ScopedH5SHandle memspace =
    H5Screate_simple(static_cast<int>(count.size()), count.data(), nullptr);
  if (memspace < 0)
  {
    vtkErrorWithObjectMacro(this->Reader, << "Error H5Screate_simple for memory space");
    return false;
  }
  // create the filespace and select the required fileExtent
  vtkHDF::ScopedH5SHandle filespace = H5Dget_space(dataset);
  if (filespace < 0)
  {
    vtkErrorWithObjectMacro(this->Reader, << "Error H5Dget_space for array");
    return false;
  }
  if (H5Sselect_hyperslab(filespace, H5S_SELECT_SET, start.data(), nullptr, count.data(), nullptr) <
    0)
  {
    std::ostringstream ostr;
    std::ostream_iterator<int> oi(ostr, " ");
    ostr << "Error selecting hyperslab, \nstart: ";
    std::copy(start.begin(), start.end(), oi);
    ostr << "\ncount: ";
    std::copy(count.begin(), count.end(), oi);
    vtkErrorWithObjectMacro(this->Reader, << ostr.str());
    return false;
  }

  // read hyperslab
  if (H5Dread(dataset, nativeType, memspace, filespace, H5P_DEFAULT, data) < 0)
  {
    vtkErrorWithObjectMacro(this->Reader, << "Error H5Dread "
                                          << "start: " << start[0] << ", " << start[1] << ", "
                                          << start[2] << " count: " << count[0] << ", " << count[1]
                                          << ", " << count[2]);
    return false;
  }

  return true;
}

bool vtkHDFReader::Implementation::ComputeAMRBlocksPerLevels(std::vector<int>& levels)
{
  levels.clear();

  if (this->DataSetType == VTK_OVERLAPPING_AMR)
  {
    unsigned int level = 0;
    bool isValidLevel = true;
    while (isValidLevel)
    {
      std::stringstream levelGroupName;
      levelGroupName << "Level" << level;
      if (H5Lexists(this->VTKGroup, levelGroupName.str().c_str(), H5P_DEFAULT) <= 0)
      {
        // The level does not exist, just exit.
        isValidLevel = false;
      }
      else
      {
        vtkHDF::ScopedH5GHandle levelGroupID =
          H5Gopen(this->VTKGroup, levelGroupName.str().c_str(), H5P_DEFAULT);
        if (levelGroupID == H5I_INVALID_HID)
        {
          vtkErrorWithObjectMacro(this->Reader, "Can't open group Level" << level);
          return false;
        }

        if (H5Lexists(levelGroupID, "AMRBox", H5P_DEFAULT) <= 0)
        {
          vtkErrorWithObjectMacro(this->Reader, "No AMRBox dataset at Level" << level);
          return false;
        }

        vtkHDF::ScopedH5DHandle amrBoxDatasetID = H5Dopen(levelGroupID, "AMRBox", H5P_DEFAULT);
        if (amrBoxDatasetID == H5I_INVALID_HID)
        {
          vtkErrorWithObjectMacro(this->Reader, "Can't AMRBox dataset at Level" << level);
          return false;
        }

        vtkHDF::ScopedH5SHandle spaceID = H5Dget_space(amrBoxDatasetID);
        if (spaceID == H5I_INVALID_HID)
        {
          vtkErrorWithObjectMacro(
            this->Reader, "Can't get space of AMRBox dataset at Level" << level);
          return false;
        }

        std::array<hsize_t, 2> dimensions;
        if (H5Sget_simple_extent_dims(spaceID, dimensions.data(), nullptr) <= 0)
        {
          vtkErrorWithObjectMacro(
            this->Reader, "Can't get space dimensions of AMRBox dataset at Level" << level);
          return false;
        }

        levels.push_back(dimensions[0]);
        ++level;
      }
    }
  }

  return true;
}

bool vtkHDFReader::Implementation::FillAMR(vtkOverlappingAMR* data,
  unsigned int maximumLevelsToReadByDefault, double origin[3],
  vtkDataArraySelection* dataArraySelection[3])
{
  if (this->DataSetType != VTK_OVERLAPPING_AMR)
  {
    vtkErrorWithObjectMacro(this->Reader,
      "Wrong data set type, expected " << VTK_OVERLAPPING_AMR
                                       << ", but got: " << this->DataSetType);
    return false;
  }

  if (!dataArraySelection)
  {
    vtkErrorWithObjectMacro(this->Reader, "NULL dataArraySelection ");
    return false;
  }

  std::vector<int> blocksPerLevels;
  if (!this->ComputeAMRBlocksPerLevels(blocksPerLevels))
  {
    return false;
  }

  if (blocksPerLevels.empty())
  {
    return true;
  }

  size_t numberOfLoadedLevels = maximumLevelsToReadByDefault == 0
    ? blocksPerLevels.size()
    : std::min(blocksPerLevels.size(), static_cast<size_t>(maximumLevelsToReadByDefault));
  data->Initialize(static_cast<int>(numberOfLoadedLevels), blocksPerLevels.data());
  data->SetOrigin(origin);
  data->SetGridDescription(VTK_XYZ_GRID);

  unsigned int level = 0;
  unsigned int maxLevel = maximumLevelsToReadByDefault > 0
    ? maximumLevelsToReadByDefault
    : std::numeric_limits<unsigned int>::max();
  bool isValidLevel = true;
  while (isValidLevel && level < maxLevel)
  {
    std::string levelGroupName = "Level" + std::to_string(level);
    if (H5Lexists(this->VTKGroup, levelGroupName.c_str(), H5P_DEFAULT) <= 0)
    {
      // The level does not exist, just exit.
      isValidLevel = false;
    }
    else
    {
      if (!ReadLevelTopology(level, levelGroupName, data, origin))
      {
        vtkErrorWithObjectMacro(this->Reader, "Can't read group Level" << level);
        return false;
      }
      ++level;
    }
  }

  isValidLevel = true;
  level = 0;
  while (isValidLevel && level < maxLevel)
  {
    std::string levelGroupName = "Level" + std::to_string(level);
    if (H5Lexists(this->VTKGroup, levelGroupName.c_str(), H5P_DEFAULT) <= 0)
    {
      // The level does not exist, just exit.
      isValidLevel = false;
    }
    else
    {
      if (!ReadLevelData(level, levelGroupName, data, dataArraySelection))
      {
        vtkErrorWithObjectMacro(this->Reader, "Can't fill group Level" << level);
        return false;
      }
      ++level;
    }
  }

  vtkAMRUtilities::BlankCells(data);

  return true;
}

bool vtkHDFReader::Implementation::ReadLevelTopology(
  unsigned int level, const std::string& levelGroupName, vtkOverlappingAMR* data, double origin[3])
{
  vtkHDF::ScopedH5GHandle levelGroupID =
    H5Gopen(this->VTKGroup, levelGroupName.c_str(), H5P_DEFAULT);
  if (levelGroupID == H5I_INVALID_HID)
  {
    vtkErrorWithObjectMacro(this->Reader, "Can't open group Level" << level);
    return false;
  }

  std::array<double, 3> spacing = { 0, 0, 0 };
  if (!this->ReadLevelSpacing(levelGroupID, spacing.data()))
  {
    vtkErrorWithObjectMacro(
      this->Reader, "Error while reading spacing attribute at level " << level);
    return false;
  }
  data->SetSpacing(level, spacing.data());

  std::vector<int> amrBoxRawData;
  if (!this->ReadAMRBoxRawValues(levelGroupID, amrBoxRawData))
  {
    vtkErrorWithObjectMacro(this->Reader, "Error while reading AMRBox at level " << level);
    return false;
  }

  if (amrBoxRawData.size() % 6 != 0)
  {
    vtkErrorWithObjectMacro(this->Reader,
      "The size of the \"AMRBox\" dataset at Level" << level << " is not a multiple of 6.");
    return false;
  }

  unsigned int numberOfDatasets = static_cast<unsigned int>(amrBoxRawData.size() / 6);
  for (unsigned int dataSetIndex = 0; dataSetIndex < numberOfDatasets; ++dataSetIndex)
  {
    int* currentAMRBoxRawData = amrBoxRawData.data() + (6 * dataSetIndex);
    vtkAMRBox amrBox(currentAMRBoxRawData);

    data->SetAMRBox(level, dataSetIndex, amrBox);
    vtkNew<vtkUniformGrid> dataSet;
    dataSet->Initialize();

    const int* lowCorner = amrBox.GetLoCorner();
    double dataSetOrigin[3] = { origin[0] + lowCorner[0] * spacing[0],
      origin[1] + lowCorner[1] * spacing[1], origin[2] + lowCorner[2] * spacing[2] };
    dataSet->SetOrigin(dataSetOrigin);

    dataSet->SetSpacing(spacing.data());
    std::array<int, 3> numberOfNodes = { 0, 0, 0 };
    amrBox.GetNumberOfNodes(numberOfNodes.data());
    dataSet->SetDimensions(numberOfNodes.data());

    data->SetDataSet(level, dataSetIndex, dataSet);
  }

  return true;
}

bool vtkHDFReader::Implementation::ReadLevelData(unsigned int level,
  const std::string& levelGroupName, vtkOverlappingAMR* data,
  vtkDataArraySelection* dataArraySelection[3])
{
  vtkHDF::ScopedH5GHandle levelGroupID =
    H5Gopen(this->VTKGroup, levelGroupName.c_str(), H5P_DEFAULT);
  if (levelGroupID == H5I_INVALID_HID)
  {
    vtkErrorWithObjectMacro(this->Reader, "Can't open group Level" << level);
    return false;
  }

  // Now read actual data - one array at a time
  std::array<const char*, 3> groupNames = { "PointData", "CellData", "FieldData" };
  for (int attributeType = 0; attributeType < 3; ++attributeType)
  {
    vtkHDF::ScopedH5GHandle groupID = H5Gopen(levelGroupID, groupNames[attributeType], H5P_DEFAULT);
    if (groupID == H5I_INVALID_HID)
    {
      // It's OK to not have groups in the file if there are no data arrays
      // for that attribute type.
      continue;
    }

    auto arrayNames = this->GetArrayNames(attributeType);
    for (const std::string& name : arrayNames)
    {
      if (!dataArraySelection[attributeType]->ArrayIsEnabled(name.c_str()))
      {
        continue;
      }

      // Open dataset
      hid_t tempNativeType = H5I_INVALID_HID;
      std::vector<hsize_t> dims;
      vtkHDF::ScopedH5DHandle datasetID =
        this->OpenDataSet(groupID, name.c_str(), &tempNativeType, dims);
      vtkHDF::ScopedH5THandle nativeType = tempNativeType;
      if (datasetID < 0)
      {
        vtkErrorWithObjectMacro(this->Reader, "Can't open array " << name);
        return false;
      }

      // Iterate over all datasets, read data and assign attribute
      hsize_t dataOffset = 0;
      hsize_t dataSize = 0;
      unsigned int numberOfDatasets = data->GetNumberOfDataSets(level);
      for (unsigned int dataSetIndex = 0; dataSetIndex < numberOfDatasets; ++dataSetIndex)
      {
        const vtkAMRBox& amrBox = data->GetAMRBox(level, dataSetIndex);
        auto dataSet = data->GetDataSet(level, dataSetIndex);
        if (dataSet == nullptr)
        {
          vtkErrorWithObjectMacro(this->Reader,
            "Error fetching dataset at level " << level << " and dataSetIndex " << dataSetIndex);
          return false;
        }

        // Here dataSize is the size of the previous dataset read. Offset
        // is incremented, and a new size is specified after the increment.
        // This allows for reading AMR's where the size of the blocks vary
        // inside each level.
        dataOffset += dataSize;

        switch (attributeType)
        {
          case 0:
            dataSize = amrBox.GetNumberOfNodes();
            break;
          case 1:
            dataSize = amrBox.GetNumberOfCells();
            break;
          case 2:
            dataSize = dims[0] / numberOfDatasets;
        }

        std::vector<hsize_t> fileExtent = { dataOffset, dataOffset + dataSize };

        vtkSmartPointer<vtkDataArray> array;
        if ((array = vtk::TakeSmartPointer(
               this->NewArrayForGroup(datasetID, nativeType, dims, fileExtent))) == nullptr)
        {
          vtkErrorWithObjectMacro(this->Reader, "Error reading array " << name);
          return false;
        }
        array->SetName(name.c_str());
        dataSet->GetAttributesAsFieldData(attributeType)->AddArray(array);
      }
    }
  }

  return true;
}

bool vtkHDFReader::Implementation::ReadLevelSpacing(hid_t levelGroupID, double* spacing)
{
  if (!H5Aexists(levelGroupID, "Spacing"))
  {
    vtkErrorWithObjectMacro(this->Reader, "\"Spacing\" attribute does not exist.");
    return false;
  }
  vtkHDF::ScopedH5AHandle spacingAttributeID = H5Aopen_name(levelGroupID, "Spacing");
  if (spacingAttributeID < 0)
  {
    vtkErrorWithObjectMacro(this->Reader, "Can't open \"Spacing\" attribute.");
    return false;
  }

  if (H5Aread(spacingAttributeID, H5T_NATIVE_DOUBLE, spacing) < 0)
  {
    vtkErrorWithObjectMacro(this->Reader, "Can't read \"Spacing\" attribute.");
    return false;
  }

  return true;
}

bool vtkHDFReader::Implementation::ReadAMRBoxRawValues(
  hid_t levelGroupID, std::vector<int>& amrBoxRawData)
{
  if (H5Lexists(levelGroupID, "AMRBox", H5P_DEFAULT) <= 0)
  {
    vtkErrorWithObjectMacro(this->Reader, "No AMRBox dataset");
    return false;
  }

  vtkHDF::ScopedH5DHandle amrBoxDatasetID = H5Dopen(levelGroupID, "AMRBox", H5P_DEFAULT);
  if (amrBoxDatasetID == H5I_INVALID_HID)
  {
    vtkErrorWithObjectMacro(this->Reader, "Can't open AMRBox dataset");
    return false;
  }

  vtkHDF::ScopedH5SHandle spaceID = H5Dget_space(amrBoxDatasetID);
  if (spaceID == H5I_INVALID_HID)
  {
    vtkErrorWithObjectMacro(this->Reader, "Can't get space of AMRBox dataset");
    return false;
  }

  std::array<hsize_t, 2> dimensions;
  if (H5Sget_simple_extent_dims(spaceID, dimensions.data(), nullptr) <= 0)
  {
    vtkErrorWithObjectMacro(this->Reader, "Can't get space dimensions of AMRBox dataset");
    return false;
  }

  if (dimensions[1] != 6)
  {
    vtkErrorWithObjectMacro(
      this->Reader, "Wrong AMRBox dimension, expected 6, got: " << dimensions[1]);
    return false;
  }

  hsize_t numberOfDatasets = dimensions[0];
  amrBoxRawData.resize(numberOfDatasets * 6);
  if (H5Dread(
        amrBoxDatasetID, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, amrBoxRawData.data()) < 0)
  {
    vtkErrorWithObjectMacro(this->Reader, "Can't read AMRBox dataset.");
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
vtkDataArray* vtkHDFReader::Implementation::GetStepValues()
{
  if (this->File < 0)
  {
    vtkErrorWithObjectMacro(this->Reader, "Cannot get step values if the file is not open");
  }
  return this->GetStepValues(this->VTKGroup);
}

//------------------------------------------------------------------------------
vtkDataArray* vtkHDFReader::Implementation::GetStepValues(hid_t group)
{
  if (group < 0)
  {
    vtkErrorWithObjectMacro(this->Reader, "Cannot get step values from empty group");
  }

  if (H5Lexists(group, "Steps", H5P_DEFAULT) <= 0)
  {
    // Steps group does not exist
    return nullptr;
  }

  // Steps group does exist
  vtkHDF::ScopedH5GHandle steps = H5Gopen(group, "Steps", H5P_DEFAULT);
  if (steps < 0)
  {
    vtkErrorWithObjectMacro(this->Reader, "Could not open steps group");
    return nullptr;
  }

  std::vector<hsize_t> fileExtent;
  return this->NewArrayForGroup(steps, "Values", fileExtent);
}

//------------------------------------------------------------------------------
vtkIdType vtkHDFReader::Implementation::GetArrayOffset(
  vtkIdType step, int attributeType, std::string name)
{
  if (this->VTKGroup < 0)
  {
    return -1;
  }
  if (H5Lexists(this->VTKGroup, "Steps", H5P_DEFAULT) <= 0)
  {
    return -1;
  }
  std::string path = "Steps/";
  path += ::ARRAY_OFFSET_GROUPS.at(attributeType);
  if (H5Lexists(this->VTKGroup, path.c_str(), H5P_DEFAULT) <= 0)
  {
    return -1;
  }
  path += "/" + name;
  if (H5Lexists(this->VTKGroup, path.c_str(), H5P_DEFAULT) <= 0)
  {
    return -1;
  }
  std::vector<vtkIdType> buffer = this->GetMetadata(path.c_str(), 1, step);
  if (buffer.empty())
  {
    return -1;
  }
  return buffer[0];
}

//------------------------------------------------------------------------------
// explicit template instantiation
template bool vtkHDFReader::Implementation::GetAttribute<int>(
  const char* attributeName, size_t dim, int* value);
template bool vtkHDFReader::Implementation::GetAttribute<double>(
  const char* attributeName, size_t dim, double* value);
VTK_ABI_NAMESPACE_END
