# Things that need to be done that are not immediately obvious.

## Android side
- Figure out how to make the permission behave more in line with security standards. (Note: Right now, it never asks for permission to read external storage but somehow has it anyway.)
- Give the app a proper name

## Native side
- Make file loaders load variables and use those instead of hardcoding the variable names.
- Consider doing more SIMD instructions in the shaders (i.e. on the GPU)
- Refactor transformations (into class or such)
- Make sure buffers are properly cleaned up
- Refactor GLShaderManager (or at least the name)
- Some vectors have slightly wrong colors
- Make sure the particle color doesn't interfere with the vector field color (i.e. make sure they are different)
- Re-implemented fps counter for (physical) devices
- Split the fragment shader
- Take into account the device screen's aspect ratio
- Implement parallelism
- Separate vector_field's functionality into display and physics
- Finish implementing the advection equation model

## Current bottlenecks
- Loading relevant data into the app - currently from phone's storage which is not ideal for big files.
- Loading biG files is slow, i.e. requires a "long" initial load time.

## Stuff to research
- instanced rendering
- geometry shader
- depth sorting computer graphics
- generate geometry in geometry shader

## General project
- Add comments
- Document the code
- Add flow diagrams mayhaps

## Next meeting points
- The field is currently static => should be changing, right ?
- Can all the (17GB) be stored on the device directly? (Technically can be on a USB without much change ig) 
- Particle interactions (?)

### Progress
- Looked into VTK for android (not a good option)
- Fully switched to opengl es 3.2 (to have access to geometry shaders)
- Implemented orthogonal projection (and other transformation) for the 3D displaying
- Geometry shader for the colouring of the vectors
- User input (touch)
- Worked on the physics of the model
- Code refactoring + initial documentation
