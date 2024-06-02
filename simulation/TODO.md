# Things that need to be done that are not immediately obvious.

## Android side
- Remove old functions
- Add/check error handling

## Native side
- Delete (old) functions ? - 2d, before compute shaders etc...
- Add/check error handling
- Refactor initial transforms in `transforms.cpp`

## Current bottlenecks


## Stuff to research
- best color wheel for scientific visualization of the direction of a 2d vector field
- Try cubicles after all
- Direct volume rendering / volume ray casting
- speed up paper Kehl (read, speed up + graphing log-log)
- Compare with older option
- time measurements


## General project
- Move diagrams to uml
- Sort out references from used code
- Figure out why lines are not continues (i.e. have gaps) - float precision? fix?
- **Fork previous versions (sequential, pool of threads) into separate branches**
- Make sure to use same case


## Next meeting points
- No lag (correctly switching buffers in cpu now, GPU loading happens in a second thread (p.i.t.a.))
- Loading init. positions from a file implemented, but won't be used for taking the measurements
- Current opengl setup (with fancy off thread buffer loading etc.) does not benefit from instanced rendering
- Refactoring done

### Progress
- 10'000 particles:
  - Sequential: 30 fps
  - Parallel: 60 fps
  - Pool of threads: 100 fps
  
- Compute shaders: 100 fps for 250'000 particles
