# VTK::RenderingWebGPU
## vtkRenderingWebGPU - WebGPU backend for rendering

### Description

This module contains the WebGPU native backend for `RenderingCore`. At the moment, only polygonal geometry can be rendered in different representations with point/cell scalar mapped colors.

#### Available features
Here is a list of currently implemented features:
1. Polygonal geometry rendering with point, line and triangle primitives.
2. Point scalar mapped coloring of surfaces.
3. Cell scalar mapped coloring.
4. Draw actors with the actor representation = `VTK_POINTS`, `VTK_WIREFRAME`, `VTK_SURFACE` and `VTK_SURFACE` with edge visibility.
5. Lighting based on VTK headlights and point/cell normals.
6. Point size adjustments.
7. Line width adjustments for wireframe and surface with edges.
8. `vtkSDL2WebGPURenderWindow` is a reference implementation of `vtkWebGPURenderWindow` that works on WebAssembly and desktop.
9. Depth testing.

#### Future work
Since WebGPU is already an abstraction over graphics APIs, this module doesn't create another level of abstraction. It uses WebGPU's C++ flavor
for it's object-oriented API and RAII. There are helper classes in the `vtkWebGPUInternals...` files for convenience and to make the bind group
initialization code look clean.

A lot of work remains to be done. Selections, volume mappers, textures, dual-depth peeling, fancy lights, platform native render windows are few that come to mind.

#### References
Here are some very interesting references to learn WebGPU from examples if you prefer code over spec.
1. https://toji.github.io/webgpu-gltf-case-study/
  A case-study that slowly builds up an efficient gltf renderer in WebGPU using javascript. The author describes downfalls in
  certain methods and proposes alternative ways when applicable.
2. https://github.com/samdauwe/webgpu-native-examples
  A curated list of single file examples if you want to see how to do X with Y like constraints using WebGPU C API.
3. https://eliemichel.github.io/LearnWebGPU/index.html
  Similar to LearnOpenGL or the vulka-tutorial.com. Walks you through getting a window, triangle, buffers, textures and 3D rendering.
  This tutorial has good coverage and the author provides a simple to use WebGPU C++ distribution.
4. https://sotrh.github.io/learn-wgpu/
  A very nice coverage of the beginner concepts of webgpu. This tutorial uses wgpu.rs
5. https://alain.xyz/blog/raw-webgpu
  Another small tutorial that lets you break the ice with WebGPU and get comfy with the concepts. This tutorial targets javascript API.
6. https://carmencincotti.com/2022-12-19/how-to-render-a-webgpu-triangle-series-part-three-video/
  A detailed, yet fun to read explanation of the swapchain and image presentation process. The author has several other
  targeted posts on WebGPU concepts.
7. https://webgpu.rocks/
  You want to look at the WebGPU API, but are afraid of reading the spec and do not want to read C headers. This website
  presents the WebGPU API and WGSL summary in a fancy way with syntax highlights.

Finally, for wgsl, the spec does a good job https://www.w3.org/TR/WGSL/


#### How to build VTK with Dawn (Highly experimental)

Things you'll need:
  1. git
  2. [depot_tools](http://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up)
  3. [SDL2](https://github.com/libsdl-org/SDL/releases/tag/release-2.26.5)

This module uses Dawn-C++ WebGPU implementation when VTK is built outside emscripten. First grab [Dawn](https://dawn.googlesource.com/dawn/) and follow their
build instructions using `gn`, not CMake.

Dawn uses the Chromium build system and dependency management so you need to install [depot_tools](http://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up) and add it to the PATH.

As of now, this module uses the SDL2 render window and interactor for desktop. SDL 2.0.18 or higher is recommended. Ubuntu 20.04
ships with SDL 2.0.10. If you're on Ubuntu 20.04, please build SDL2 from source [SDL2 build on linux](https://wiki.libsdl.org/SDL2/Installation#linuxunix) or grab a release from [SDL/releases](https://github.com/libsdl-org/SDL/releases/tag/release-2.26.5)

```sh
# Clone the repo as "dawn"
git clone https://dawn.googlesource.com/dawn dawn && cd dawn

# Bootstrap the gclient configuration
cp scripts/standalone.gclient .gclient

# Fetch external dependencies and toolchains with gclient
gclient sync
```

##### Build Dawn with `gn` and Ninja

```sh
mkdir -p out/Debug
gn gen out/Debug
autoninja -C out/Debug
```

##### Configure and build VTK

```sh
$ cmake \
-S /path/to/vtk/src \
-B /path/to/vtk/build \
-GNinja \
-DVTK_ENABLE_WEBGPU=ON \
-DVTK_USE_SDL2=ON \
-DDAWN_SOURCE_DIR=/path/to/dawn/src \
-DDAWN_BINARY_DIR=/path/to/dawn/src/out/Debug

$ cmake --build
```

##### Run the WebGPU tests
These are not regression tested with image comparisons.
```sh
$ export VTK_WINDOW_BACKEND=SDL2
$ ./bin/vtkRenderingWebGPUCxxTests
Available tests:
  0. TestCellScalarMappedColors
  1. TestConesBenchmark
  2. TestLineRendering
  3. TestPointScalarMappedColors
  4. TestSurfacePlusEdges
  5. TestTheQuad
  6. TestTheQuadPointRepresentation
  7. TestTheQuadWireframeRepresentation
  8. TestTheTriangle
  9. TestTheTrianglePointRepresentation
 10. TestTheTriangleWireframeRepresentation
 11. TestVertexRendering
 12. TestWireframe
```

##### Run the Rendering Core tests
Very few of these pass.
```sh
$ export VTK_WINDOW_BACKEND=SDL2
$ export VTK_GRAPHICS_BACKEND=WEBGPU
$ ./bin/vtkRenderingCoreCxxTests
```
