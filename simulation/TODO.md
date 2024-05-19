# Things that need to be done that are not immediately obvious.

## Android side
- Reactor the file picking - the separation of concerns is not really separating
- Add/check error handling

## Native side
- Fix logs always saying `native-lib`
- Make file loaders load variables and use those instead of hardcoding the variable names.
- Refactor transformations from `mainview` (into class or such)
- Make sure ALL buffers are properly cleaned up
- Make sure ALL no longer necessary temp files get deleted
- Refactor the functions in `Mainview` into shorter functions 
- Make sure the particle color doesn't interfere with the vector field color (i.e. make sure they are different)
- Refactor fps counter for (physical) devices
- Split the fragment shader
- Take into account the device screen's aspect ratio
- Implement parallelism (for the vector field)
- Separate vector_field's functionality into display and physics
- Add `physics object` or some other hierarchy to the physics engine
- Do not forget the interpolation for vertices - Is simply splitting between cubes sufficient ? -> Maybe ask this
- Looked into why all init function are getting called twice (something with onSurfaceCreated probs.)
- Delete (old) 2D functions ?
- Check them float checks for zero (just to be sure)
- Refactor class includes
- Add/check error handling
- Still a small lag when switching time steps
- Make particle size a uniform (fragment shader)
- Put glm into libs folder ?
- Check / correct the timing (fps, dt, etc.)
- Choose which threadpool library to use
- Check if the implementation of `velocityField` correctly handles the z-coordnate
- Refactor `dispatchComputeShader`, it has too much dependency on VectorField
- Play around with the number of groups in the compute shader
- Figure out what to do about performance going to shit over time

## Current bottlenecks
- Memory management (sisgev at large number of particles) in parallel approach

## Stuff to research
- best color wheel for scientific visualization of the direction of a 2d vector field

## General project
- Add comments
- Document the code
- Add flow diagrams mayhaps
- PREPARE FOR MIDTERM MEETING!!!

## Next meeting points

### Progress
- Color wheel changed
- Dynamic vector field file loading
- Fluid vector field not static anymore
- Added initial processing of files to be done in parallel
- Added vector field time linear interpolation
- 10'000 particles:
  - Sequential: 30 fps
  - Parallel: 60 fps
  - Pool of threads: 100 fps
  
- Compute shaders: 90 fps for 250'000 particles
