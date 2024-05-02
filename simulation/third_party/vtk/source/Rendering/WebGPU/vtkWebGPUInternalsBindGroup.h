// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause

#ifndef vtkWebGPUInternalsBindGroup_h
#define vtkWebGPUInternalsBindGroup_h

#include "vtkRenderingWebGPUModule.h"
#include "vtk_wgpu.h"

#include <initializer_list>

VTK_ABI_NAMESPACE_BEGIN
class VTKRENDERINGWEBGPU_EXPORT vtkWebGPUInternalsBindGroup
{
public:
  // Helpers to make creating bind groups look nicer:
  //
  //   vtkWebGPUInternalsBindGroup::MakeBindGroup(
  //       device,
  //       layout,
  //   {
  //       {0, mySampler},
  //       {1, myBuffer, offset, size},
  //       {3, myTextureView}
  //   });

  // Structure with one constructor per-type of bindings, so that the initializer_list accepts
  // bindings with the right type and no extra information.
  struct BindingInitializationHelper
  {
    BindingInitializationHelper(uint32_t binding, const wgpu::Sampler& sampler);
    BindingInitializationHelper(uint32_t binding, const wgpu::TextureView& textureView);
    BindingInitializationHelper(uint32_t binding, const wgpu::Buffer& buffer, uint64_t offset = 0,
      uint64_t size = wgpu::kWholeSize);
    BindingInitializationHelper(const BindingInitializationHelper&);
    ~BindingInitializationHelper();

    wgpu::BindGroupEntry GetAsBinding() const;

    uint32_t binding;
    wgpu::Sampler sampler;
    wgpu::TextureView textureView;
    wgpu::Buffer buffer;
    uint64_t offset = 0;
    uint64_t size = 0;
  };

  static wgpu::BindGroup MakeBindGroup(const wgpu::Device& device,
    const wgpu::BindGroupLayout& layout,
    std::initializer_list<BindingInitializationHelper> entriesInitializer);
};
VTK_ABI_NAMESPACE_END

#endif
// VTK-HeaderTest-Exclude: vtkWebGPUInternalsBindGroup.h
