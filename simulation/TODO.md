# Things that need to be done that are not immediately obvious.

## Android side
- Refactor unit tests ?

## Native side
- Create a common super class for timers
- Add timers to main

## Current bottlenecks
For a lot of particles, or randomly distributed particles in a dense grid, the bottleneck is
the grid interpolation, as a lot of different vertices are being accessed, causing cache misses
and lowering throughput. The compute shaders (best) approach might benefit from a dynamic
grid-wise parallelization.
- Rendering is the biggest bottleneck
- For thread pool parallel - Small number of particles cause crashes 


## Stuff to research
- speed up paper Kehl (read, speed up + graphing log-log); Relevant pages: 10, 
- Retake measurements:
  - sequential:
    - 
  - Parallel:
    - 5
    - 25
    - 625

- Try cubicles after all
- Direct volume rendering / volume ray casting

## General project
- Move diagrams to uml
- Sort out references from used code
- Make sure to use same case (e.g. camelCase)
- See what's is up with the parallel implementation for low particles:
```logcat
2024-06-12 19:48:02.773 23479-23479 libc++abi               com.rug.lagrangianfluidsimulation    E  terminating with uncaught exception of type netCDF::exceptions::NcHdfErr: NetCDF: HDF error
                                                                                                    file: ncFile.cpp  line:88
2024-06-12 19:48:02.773 23479-23479 libc                    com.rug.lagrangianfluidsimulation    A  Fatal signal 6 (SIGABRT), code -1 (SI_QUEUE) in tid 23479 (fluidsimulation), pid 23479 (fluidsimulation)
2024-06-12 19:48:02.903  1477-1477  audit                   auditd                               E  type=1400 audit(1718214482.901:755): avc:  denied  { read } for  pid=25388 comm="crash_dump64" path="/data/data/com.rug.lagrangianfluidsimulation/tmp/tempU.nc" dev="dm-58" ino=245228 scontext=u:r:crash_dump:s0:c157,c257,c512,c768 tcontext=u:object_r:app_data_file:s0:c157,c257,c512,c768 tclass=file permissive=0 SEPF_SM-S918B_13_0001 audit_filtered
2024-06-12 19:48:02.904  1477-1477  audit                   auditd                               E  type=1400 audit(1718214482.901:756): avc:  denied  { write } for  pid=25388 comm="crash_dump64" path="/data/data/com.rug.lagrangianfluidsimulation/tmp/tempV.nc" dev="dm-58" ino=381919 scontext=u:r:crash_dump:s0:c157,c257,c512,c768 tcontext=u:object_r:app_data_file:s0:c157,c257,c512,c768 tclass=file permissive=0 SEPF_SM-S918B_13_0001 audit_filtered
```
- Update the main TODO.md from the new entries from this branch's TODO.md
- Update `updateParticlesPool`, and `threadPool.h` fix to main -> Solves the parallel implementation for low particles so also update todo

## Next meeting points
- No lag (correctly switching buffers in cpu now + GPU buffer loading happens in a second thread (p.i.t.a.))
- Loading init. positions from a file implemented, but wasn't used be used for taking the measurements
- Current opengl setup (with fancy off thread buffer loading etc.) does not benefit from instanced rendering - it would actually be less efficient
- 
- Refactoring done
- Added mode enum. for easy switching between sequential, parallel, and compute shaders modes
- Implemented unit tests
- 
- Figured out why lines were not continuous (Julian question) - "bug" in grid interpolation (it was not)
-
- Bring up the newly discovered bottleneck...
- 
- Read the timing-relevant papers (mainly the speedup  one)
- Implemented an automated bash script that takes all the measurements
- Ran it for 3 hours to get the measurements => graphs
- 
- Question: How much of the proposal am I expected / is acceptable to re-use for the thesis? (abstract, intro., ...)
- Question: Should I also measure for < 1500 particles to "measure overhead"?
- Question: The logs for sequential - lot of particles are sparse cause it takes 30 seconds to do a single time step, do something about that ?
- Question: Frame-rate is capped at 120 fps because of v-sync, and it is seems quite annoying to try to bypass, is it an issue ?
- Question: Simulation step is always called just before rendering, thus I measure time between draw() calls, is that okay ?
- Question: Since a lot of the app is asynchronous, for compute shaders especially cause lot of GPU time, is it okay to just limit (unrelated) background processes and measure the wall-clock time?

### Perlin stuff
- Particle are moving in the z direction so app works (triple checked)
- Even in Perlin tho, particles like being glued to the walls, as the z-velocities don't switch so often
- Also takes quite some time before they switch at all for the first time
- Yes until step around 15-18
- Then basically stuck to the wall until step 45, then nice until 55-ish
- Strong convergence to a corner around step 70

### Progress
- 10'000 particles:
  - Sequential: 30 fps
  - Parallel: 60 fps
  - Pool of threads: 100 fps
  
- Compute shaders: 100 fps for 250'000 particles
