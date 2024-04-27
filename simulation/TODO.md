# Things that need to be done that are not immediately obvious.

## Android side
- [] Figure out how to make the permission behave more in line with security standards. (Note: Right now, it never asks for permission to read external storage but somehow has it anyway.)

## Native side
- [] Refactor the code into classes.
- [] Consider doing more SIMD instructions in the shaders (i.e. on the GPU)

## Current bottlenecks
- [] Loading relevant data into the app - currently from phone's storage which is not ideal for big files.
- [] Loading bif files is slow, i.e. requires a "long" initial load time.

## Next meeting points
- [] Make sure about the dimensions / use of the 3D data.
- [] Ask about OpenGL ES 3 or 2 - idk which one is better suitable. Internet seems to think 3 is better, but 2 seemed to perform better (+ is better supported ?)
- [] Should I use netcdf or hdf5 files ? (So far I have been using netcdf and it was nice, so if it is possible to keep using this... +++)
- [] Does the final app have to be 3D ? If so, how do I make sure the data is displayed correctly in 3D ? 