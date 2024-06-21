# Things that need to be done that are not immediately obvious.

## Android side


## Native side
- Create a common super class for timers


## General project
- Retake measurements:
  - sequential:
    - 3'500.7 (or check cont7)
  - Parallel:
  - Comp. shaders:
    - 2'200'000
    - 5'000'000

- Make the 1.measurement.plan file  be comprehensible by other humans
    
- Take some more measurements for cycles with loading such that I can do a [stacked bar chart](https://matplotlib.org/stable/gallery/lines_bars_and_markers/bar_stacked.html), i.e. take measurements where load
time is included asynchroneously as it normaly would be, but calculate the proportions
- Take dense grid wall clock time measurements
- Consider taking the measurements with the "uniform" initial position to ensure that the particles are chaotic - better showcase of the bottleneck


### Perlin stuff
- Particle are moving in the z direction so app works (triple checked)
- Even in Perlin tho, particles like being glued to the walls, as the z-velocities don't switch so often
- Also takes quite some time before they switch at all for the first time
- Yes until step around 15-18
- Then basically stuck to the wall until step 45, then nice until 55-ish
- Strong convergence to a corner around step 70

