# Things that need to be done that are not immediately obvious.

## Android side
- Reactor the file picking - the separation of concerns is not really separating
- Add/check error handling

## Native side
- Implement parallelism (for the vector field)
- Separate vector_field's functionality into display and physics
- Add `physics object` or some other hierarchy to the physics engine
- Do not forget the interpolation for vertices - Is simply splitting between cubes sufficient ? -> Maybe ask this
- Looked into why all init function are getting called twice (something with onSurfaceCreated probs.)
- Delete (old) 2D functions ?
- Check them float checks for zero (just to be sure)
- Refactor class includes
- Add/check error handling
- Refactor `dispatchComputeShader`, it has too much dependency on VectorField
- Remove `emojiCompat` native class
- Shaders+source in `mainview` do not need to be a class-scoped variables
- Check for hardcoded field dimensions
- Refactor initial transforms in `transforms.cpp`
- Refactor the placement of `.cpp` and `.h`/`.hpp` files
- Look at timestep (dt etc.) before starting measuring
- `loadTimeStep` and `loadPositionFromFile` are similar and could be refactored

## Current bottlenecks


## Stuff to research
- Instanced rendering, details
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
- Figure out why lines are not continues (i.e. have gaps) - float precision? fix?
- Loading initial positions from file

## Next meeting points
- No lag (correctly switching buffers in cpu now, GPU loading happens in a second thread (p.i.t.a.))
- Loading init. positions from a file implemented, but won't be used for taking the measurements

### Progress
- 10'000 particles:
  - Sequential: 30 fps
  - Parallel: 60 fps
  - Pool of threads: 100 fps
  
- Compute shaders: 100 fps for 250'000 particles
