# Things that need to be done that are not immediately obvious.

## Android side
- [] Figure out how to make the permission behave more in line with security standards. (Note: Right now, it never asks for permission to read external storage but somehow has it anyway.)

## Native side
- [] Refactor the code into classes.
- [] Consider doing more SIMD instructions in the shaders (i.e. on the GPU)

## Current bottlenecks
- [] Loading relevant data into the app - currently from phone's storage which is not ideal for big files.
- [] Loading bif files is slow, i.e. requires a "long" initial load time.