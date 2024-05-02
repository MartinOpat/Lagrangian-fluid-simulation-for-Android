// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
/**
 * @class   vtkCompositedSynchronizedRenderers
 * @brief   vtkCompositedSynchronizedRenderers is vtkSynchronizedRenderers that uses
 * vtkCompositer to composite the images on the root node.
 */

#ifndef vtkCompositedSynchronizedRenderers_h
#define vtkCompositedSynchronizedRenderers_h

#include "vtkRenderingParallelModule.h" // For export macro
#include "vtkSynchronizedRenderers.h"

VTK_ABI_NAMESPACE_BEGIN
class vtkFloatArray;
class vtkCompositer;

class VTKRENDERINGPARALLEL_EXPORT vtkCompositedSynchronizedRenderers
  : public vtkSynchronizedRenderers
{
public:
  static vtkCompositedSynchronizedRenderers* New();
  vtkTypeMacro(vtkCompositedSynchronizedRenderers, vtkSynchronizedRenderers);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///@{
  /**
   * Get/Set the composite. vtkTreeCompositer is used by default.
   */
  void SetCompositer(vtkCompositer*);
  vtkGetObjectMacro(Compositer, vtkCompositer);
  ///@}

protected:
  vtkCompositedSynchronizedRenderers();
  ~vtkCompositedSynchronizedRenderers() override;

  void MasterEndRender() override;
  void SlaveEndRender() override;
  void CaptureRenderedDepthBuffer(vtkFloatArray* depth_buffer);

  vtkCompositer* Compositer;

private:
  vtkCompositedSynchronizedRenderers(const vtkCompositedSynchronizedRenderers&) = delete;
  void operator=(const vtkCompositedSynchronizedRenderers&) = delete;
};

VTK_ABI_NAMESPACE_END
#endif
