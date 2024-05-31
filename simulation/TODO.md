# Things that need to be done that are not immediately obvious.

## Android side
- Reactor the file picking - the separation of concerns is not really separating
- Add/check error handling
~~- Do not reset at screen rotation?~~ Activity gets destroyed and recreated on rotation, not important enough

## Native side
- Make file loaders load variables and use those instead of hardcoding the variable names (?)
- Make sure the particle color doesn't interfere with the vector field color (i.e. ask supervisor if good or to give specific reference)
- Refactor fps counter for (physical) devices?
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
- Refactor `dispatchComputeShader`, it has too much dependency on VectorField
- Play around with the number of groups in the compute shader
- Figure out what to do about performance going to shit over time
- Remove `emojiCompat` native class
- Check whether to close AssetManager immediately after use (see `png_loader`)
- Shaders+source in `mainview` do not need to be a class-scoped variables
- Check for hardcoded field dimensions
- Refactor initial transforms in `transforms.cpp`

## Current bottlenecks
- Memory management (sisgev at large number of particles) in parallel approach

## Stuff to research
- best color wheel for scientific visualization of the direction of a 2d vector field
- Try cubicles after all
- Direct volume rendering / volume ray casting
- speed up paper Kehl (read, speed up + graphing log-log)
- Compare with older option
- time measurements


## General project
- Add comments
- Document the code
- Move diagrams to uml
- Sort out references from used code

## Next meeting points

### Progress
- 10'000 particles:
  - Sequential: 30 fps
  - Parallel: 60 fps
  - Pool of threads: 100 fps
  
- Compute shaders: 100 fps for 250'000 particles
