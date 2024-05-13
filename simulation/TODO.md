# Things that need to be done that are not immediately obvious.

## Android side
- Figure out how to make the permission behave more in line with security standards. (Note: Right now, it never asks for permission to read external storage but somehow has it anyway.)
- Give the app a proper name

## Native side
- Make file loaders load variables and use those instead of hardcoding the variable names.
- Consider doing more SIMD instructions in the shaders (i.e. on the GPU)
- Refactor transformations from `mainview` (into class or such)
- Make sure ALL buffers are properly cleaned up
- Refactor `GLShaderManager` (or at least the name)
- Make sure the particle color doesn't interfere with the vector field color (i.e. make sure they are different)
- Re-implemented fps counter for (physical) devices
- Split the fragment shader
- Take into account the device screen's aspect ratio
- Implement parallelism
- Separate vector_field's functionality into display and physics
- Finish implementing the advection equation model
- Add `physics object` or some other hierarchy to the physics engine
- Do not forget the interpolation for vertices
- 

## Current bottlenecks
- Loading relevant data into the app - currently from phone's storage which is not ideal for big files.
- Loading biG files is slow, i.e. requires a "long" initial load time.

## Stuff to research
- instanced rendering
- depth sorting computer graphics
- generate geometry in geometry shader

## General project
- Add comments
- Document the code
- Add flow diagrams mayhaps

## Next meeting points
- Is the way of 3D displaying sufficient?
- Add edges to better see the vector field ?
- ~~ The field is currently static => should be changing, right ? ~~
- ~~Can all the (17GB) be stored on the device directly? (Technically can be on a USB without much change ig) ~~
- Add particle interactions ? -> No

### Progress
- Looked into VTK for android (not a good option)
- Fully switched to opengl es 3.2 (to have access to geometry shaders)
- Implemented orthogonal projection (and other transformation) for the 3D displaying
- Geometry shader for the colouring of the vectors
- User input (touch)
- Worked on the physics of the model -> two kinds
- Code refactoring + initial documentation
