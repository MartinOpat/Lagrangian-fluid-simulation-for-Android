# Things that need to be done that are not immediately obvious.

## Android side
- UPDATE THIS BRANCH WITH THE TODAY'S CODE ENHANCEMENTS IN ORDER TO MEASURE THE DENSE/SPARSE GRID PERFORMANCE


## Native side
- Create a common super class for timers


## General project
- Retake measurements:
  - sequential:
    - 
  - Parallel:


### Perlin stuff
- Particle are moving in the z direction so app works (triple checked)
- Even in Perlin tho, particles like being glued to the walls, as the z-velocities don't switch so often
- Also takes quite some time before they switch at all for the first time
- Yes until step around 15-18
- Then basically stuck to the wall until step 45, then nice until 55-ish
- Strong convergence to a corner around step 70

