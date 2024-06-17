## To discuss in thesis
- Instanced rendering not beneficial for current setup
- research objective and res. question should be in intro; Make a contribution subsection.
- Describe new approach and how to design experiments (methods section) -> explain  how testing what I did answers R.Q.
- discussion -> discuss exp., impact, (loading different data, features, interactions, what can be done), future work (tried out things, bottlenecks, load distrib.)
- Describe required dataset(s) either before methods or very first in methods
- Reuse abstract and intro from proposal
- background section can be based on state-of-the-art but needs updating to be relevant
- Discuss capped frame-rate due to v-sync and its impact on the measurements in discussion sections

## Android side
- Delete unused project files / folders
- Fixed the occasional glitch during user interactions

## Native side
- Delete unused functions

## Current bottlenecks
For a lot of particles, or randomly distributed particles in a dense grid, the bottleneck is
the grid interpolation, as a lot of different vertices are being accessed, causing cache misses
and lowering throughput.
The compute shaders (best) approach might benefit from a dynamic
grid-wise parallelization -> Not necessarily.
-> Could simply due to the fact that the particles in thread-groups are far apart in memory -> No simple fix
- -> Read about warps and global memory
- -> Discuss the CPU equivalent
Note that the dense grid is pretty much an issue just for the compute shaders, for the other two modes it is negligable

I tried measuring the time development of the app, to see how the individual CPU/GPU times develop over time. I have noticed
a disrepancy between the total CPU/GPU times and the wall clock time, as the measured values for the CPU or GPU times did not actually 
seem to include the buffer-syncing bottle neck. I have looked into this using the profiler (and several other similar debug tools). 
And I arrived at the following (quite tricky) conclusion:

The whole things above is actually buffer sync and swap between the particle SSAO buffer in the compute shader 
and the particle VBO/VAO buffer used for rendering. As the EGL context is handled (and thus also synced) by java, there
is no opengl call corresponding particularly to this inherent syncing. Thus, since it is in java, without opengl calls, 
the gl time query in native cannot time it.
I tried to re-implement the whole app using NativeActivity instead of MainActivity so that the total control
of the app (incuding event handling and egl context) would be on the native side. And while I did manage to imeplement a quick
bare-bones implementation of this approach, it was not stable enough to actually take the  measurements. It would need quite a lot of time
to basically refactor the android-native communication bridge from sratch, i.e. not feasible for this project.
The buffer swapping visible in the wall clock time is inherently not going to be measured and reflected by the CPU/GPU time queries.
For this reason, I think it is going to be beneficial to include the wall clock time measurements directly in the thesis, as the other
purely GPU/CPU time measurements are misleading in this regard.

While poking around into possible ways how to either measure, eliminate, or minimize this buffer-sync bottleneck, at one point I made the decision
to change the scaling of the vector fields components. This actually had the nice "side-effect" that it slightly improved the "boring" z-component
of the double gyre field, as the w-velocity were scaled down so particles take longer to reach the boundaries of the field.

I have also decided that due to the above bottleneck it would be beneficial to esentially take two different types/sets of measurements
1. Dense grid -> This would be the full $$\approx~500 \cdot 250 \cdot 28~\approx~10^6$$ vertices of the double gyre field
2. Sparse grid -> This would be only $$\approx 30 \cdot 16 \cdot 5 \approx 10^3 $$ vertices of the double gyre field - basically the same set I used for rendering the field
To not waste previous measurements, the double gyre wll be used in both sets with the same initial position of the particles, which is the diagonal line. That way, I missing 
only 1 or 2 runs, which I can take today while writing thesis. 

After all this profiling and debuggin that I did over the weekend to figure this whole thing out, I have gained quite a lot of insight into how exactly the app works, so I am 
quite excited to write about this in the thesis. I plan to have the first draft on wednesday, will see how it goes.


## Stuff to research
- Try cuberiles after all
- Direct volume rendering / volume ray casting
- The outline box approach

## General project
- Sort out references from used code
- Make sure to use same case (e.g. camelCase)
- Try better rendering methods when exatra time, e.g. thesis draft is being review ->volume rendering / volume ray casting, find new ones
- Mention the different (attempted) different ways of rendering stuff - so far: LIC
- Make a measurement for the "new bottleneck" see how the performance changes with a more random distrib. (either perlin, or bigger double-gyre)
- Make proper APKs for a) reduced, b) full version of the app
- Make measurements for (very) small numbers of particles -> see if v-sync doesn't make it pointless
- Question: Since a lot of the app is asynchronous, for compute shaders especially cause lot of GPU time, is it okay to just limit (unrelated) background processes and measure the wall-clock time?
  - Ideally measure it all separately, and report that way.
  - Measure, GPU compute, GPU render, GPU buffer load, CPU file load, CPU time ... all separately, as any could potentially become bottleneck
- Update main readme with gradlew commands

- UPDATE THE MEASUREMENT BRANCH WITH THE TODAY'S CODE ENHANCEMENTS IN ORDER TO MEASURE THE DENSE/SPARSE GRID PERFORMANCE

## Next meeting points

