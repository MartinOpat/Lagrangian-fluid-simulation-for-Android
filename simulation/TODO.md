# Things that need to be done that are not immediately obvious.

## Android side
- Refactor unit tests ?

## Native side

## Current bottlenecks


## Stuff to research
- speed up paper Kehl (read, speed up + graphing log-log)
- time measurements for all three modes

- Try cubicles after all
- Direct volume rendering / volume ray casting

## General project
- Move diagrams to uml
- Sort out references from used code
- Figure out why lines are not continues (i.e. have gaps) - float precision? fix?
- Make sure to use same case (e.g. camelCase)


## Next meeting points
- No lag (correctly switching buffers in cpu now, GPU loading happens in a second thread (p.i.t.a.))
- Loading init. positions from a file implemented, but won't be used for taking the measurements
- Current opengl setup (with fancy off thread buffer loading etc.) does not benefit from instanced rendering - it is actually less efficient
- Refactoring done
- Added mode enum. for easy switching between sequential, parallel, and compute shaders modes
- Implemented unit tests
- Question: How much of the proposal am I expected to re-use for the thesis / is acceptable? (abstract, intro., ...)

### Progress
- 10'000 particles:
  - Sequential: 30 fps
  - Parallel: 60 fps
  - Pool of threads: 100 fps
  
- Compute shaders: 100 fps for 250'000 particles
