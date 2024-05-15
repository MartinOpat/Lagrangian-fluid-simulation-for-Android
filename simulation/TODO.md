# Things that need to be done that are not immediately obvious.

## Android side
- Figure out how to make the permission behave more in line with security standards. (Note: Right now, it never asks for permission to read external storage but somehow has it anyway.)
- Give the app a proper name
- pick code consts. should be in one place only
- Reactor the file picking - the separation of concerns is not really separating

## Native side
- Interpolate between time steps
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
- Add `physics object` or some other hierarchy to the physics engine
- Do not forget the interpolation for vertices
- Looked into why all init function are getting called twice (something with onSurfaceCreated probs.)
- Delete (old) 2D functions

## Current bottlenecks
- Loading relevant data into the app - currently from phone's storage which is not ideal for big files.
- Loading biG files is slow, i.e. requires a "long" initial load time.

## Stuff to research
- best color wheel for scientific visualization of the direction of a 2d vector field

## General project
- Add comments
- Document the code
- Add flow diagrams mayhaps

## Next meeting points

### Progress

