// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
/**
 * @class   vtkPResampleToImage
 * @brief   sample dataset on a uniform grid in parallel
 *
 * vtkPResampleToImage is a parallel filter that resamples the input dataset on
 * a uniform grid. It internally uses vtkProbeFilter to do the probing.
 * @sa
 * vtkResampleToImage vtkProbeFilter
 */

#ifndef vtkPResampleToImage_h
#define vtkPResampleToImage_h

#include "vtkFiltersParallelDIY2Module.h" // For export macro
#include "vtkResampleToImage.h"

VTK_ABI_NAMESPACE_BEGIN
class vtkDataSet;
class vtkImageData;
class vtkMultiProcessController;

class VTKFILTERSPARALLELDIY2_EXPORT vtkPResampleToImage : public vtkResampleToImage
{
public:
  vtkTypeMacro(vtkPResampleToImage, vtkResampleToImage);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  static vtkPResampleToImage* New();

  ///@{
  /**
   * By default this filter uses the global controller,
   * but this method can be used to set another instead.
   */
  virtual void SetController(vtkMultiProcessController*);
  vtkGetObjectMacro(Controller, vtkMultiProcessController);
  ///@}

protected:
  vtkPResampleToImage();
  ~vtkPResampleToImage() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  vtkMultiProcessController* Controller;

private:
  vtkPResampleToImage(const vtkPResampleToImage&) = delete;
  void operator=(const vtkPResampleToImage&) = delete;
};

VTK_ABI_NAMESPACE_END
#endif
