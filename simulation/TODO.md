# Things that need to be done that are not immediately obvious.

## Android side
- Reactor the file picking - the separation of concerns is not really separating
- Add/check error handling

## Native side
- Fix logs always saying `native-lib`
- Make file loaders load variables and use those instead of hardcoding the variable names.
- Consider doing more SIMD instructions in the shaders (i.e. on the GPU)
- Refactor transformations from `mainview` (into class or such)
- Make sure ALL buffers are properly cleaned up
- Refactor the functions in `Mainview` into shorter functions 
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
- Check them float checks for zero (just to be sure)
- Refactor class includes
- Add/check error handling
- Still a small lag when switching time steps

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

