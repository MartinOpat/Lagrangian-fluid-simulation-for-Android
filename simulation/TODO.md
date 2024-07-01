## To discuss in thesis
- Instanced rendering not beneficial for current setup
- research objective and res. question should be in intro; Make a contribution subsection.
- Describe new approach and how to design experiments (methods section) -> explain  how testing what I did answers R.Q.
- discussion -> discuss exp., impact, (loading different data, features, interactions, what can be done), future work (tried out things, bottlenecks, load distrib.)
- Describe required dataset(s) either before methods or very first in methods
- Reuse abstract and intro from proposal
- background section can be based on globalAppState-of-the-art but needs updating to be relevant
- Discuss capped frame-rate due to v-sync and its impact on the measurements in discussion sections
- 
- Discuss the discrepancies (see status update email thread)
-
- Check that the uses of "was done" and "will be done" is consistent and makes sense
- Make sure that it is properly distinguished between the time step of switching files (a day), and a simulation time step (dt)
- Make sure that it is clear that fluid simulation and particle simulation is the same thing as we are simulating the fluid as particles (probs. in the intro)


## Android side
- Delete unused project files / folders

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


## Stuff to research
- Try cuberiles after all
- Direct volume rendering / volume ray casting
- The outline box approach

## General project
- Sort out references/licenses from used code
- Make sure to use same case (e.g. camelCase)
- Try better rendering methods when exatra time, e.g. thesis draft is being review ->volume rendering / volume ray casting, find new ones
- Mention the different (attempted) different ways of rendering stuff - so far: LIC
- Make a measurement for the "new bottleneck" see how the performance changes with a more random distrib. (either perlin, or bigger double-gyre)

- Put detailed lib versions in the readme.md
- Put library credit in the readme.md
- Add apk as a github release

## Next meeting points
- Make repo public?
- When all is done, get that best thesis price (pwease) - ask Kehl for recommendation / nomination (https://khmw.nl/khmw-jong-talent-prijzen-khmw-young-talent-awards/)

