// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
#include "vtkGenerateTimeSteps.h"

#include "vtkDataObject.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"

#include <algorithm>
#include <cmath>
#include <vector>

VTK_ABI_NAMESPACE_BEGIN
vtkStandardNewMacro(vtkGenerateTimeSteps);

//------------------------------------------------------------------------------
void vtkGenerateTimeSteps::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  int count = static_cast<int>(this->TimeStepValues.size());
  os << indent << "Number of Time Steps: " << count << std::endl;
  if (count > 0)
  {
    os << indent << "Time Step Values: ";
    for (double val : this->TimeStepValues)
    {
      os << val << " ";
    }
    os << std::endl;
  }
}

//------------------------------------------------------------------------------
void vtkGenerateTimeSteps::AddTimeStepValue(double timeStepValue)
{
  this->TimeStepValues.emplace_back(timeStepValue);
  this->Modified();
}

//------------------------------------------------------------------------------
void vtkGenerateTimeSteps::SetTimeStepValues(int count, const double* timeStepValues)
{
  this->TimeStepValues.clear();
  this->TimeStepValues.insert(this->TimeStepValues.begin(), timeStepValues, timeStepValues + count);
  this->Modified();
}

//------------------------------------------------------------------------------
void vtkGenerateTimeSteps::GetTimeStepValues(double* timeStepValues) const
{
  std::copy(this->TimeStepValues.cbegin(), this->TimeStepValues.cend(), timeStepValues);
}

//------------------------------------------------------------------------------
void vtkGenerateTimeSteps::GenerateTimeStepValues(double begin, double end, double step)
{
  // overflow and infinite loop check
  double diff = begin - end;
  if (std::signbit(diff) == std::signbit(step) || step == 0)
  {
    vtkErrorMacro(<< "Incorrect range definition: begin: " << begin << " end: " << end
                  << " step: " << step);
    return;
  }

  this->TimeStepValues.clear();
  for (int i = begin; i < end; i += step)
  {
    this->TimeStepValues.emplace_back(i);
  }
  this->Modified();
}

//------------------------------------------------------------------------------
int vtkGenerateTimeSteps::RequestInformation(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  if (!this->TimeStepValues.empty())
  {
    vtkInformation* outInfo = outputVector->GetInformationObject(0);
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(), this->TimeStepValues.data(),
      static_cast<int>(this->TimeStepValues.size()));

    const auto rangePair =
      std::minmax_element(this->TimeStepValues.cbegin(), this->TimeStepValues.cend());
    double range[2] = { *rangePair.first, *rangePair.second };
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), range, 2);
  }

  return 1;
}

//------------------------------------------------------------------------------
int vtkGenerateTimeSteps::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  // Shallow copy input into output
  vtkDataObject* inData = vtkDataObject::GetData(inputVector[0], 0);
  vtkDataObject* outData = vtkDataObject::GetData(outputVector, 0);

  if (inData && outData)
  {
    outData->ShallowCopy(inData);
  }
  return 1;
}
VTK_ABI_NAMESPACE_END
