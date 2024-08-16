# Config file structure
## Overview
The `config.txt` file is a simple file from which variables get exported into native code in the CMakeLists.txt file.
Currently, no comments are supported in the `config.txt` file.
The variables can be accessed in the native code as:
```cpp
#ifdef LOAD_POSITIONS_FROM_FILE
        // Code here
#endif
```

## Description of currently exported variables
- `LOAD_POSITIONS_FROM_FILE`: Whether to load positions from a file or generate them in code via `ParticlesHandler::InitType`.
- `REDUCE_FIELD_GRAPHICS`: Whether to reduce the number of rendered field lines.
- `DOUBLE_GYRE_DEFAULT_SETTINGS`: Whether to use the physics preset for the double gyre.
- `PERLIN_DEFAULT_SETTINGS`:  Whether to use the physics preset for the perlin noise.
- `USE_GPU`: Whether to use the GPU for the calculations.
- `USE_CPU_PARALLELISM`: Whether to use multiple CPU threads for the calculations.

Setting any of the above variables to `1` will enable the feature, setting it to `0` will disable it. Note that the following sets of variables are mutually exclusive and should not be set to `1` at the same time:
- `DOUBLE_GYRE_DEFAULT_SETTINGS` and `PERLIN_DEFAULT_SETTINGS`
- `USE_GPU` and `USE_CPU_PARALLELISM`

Setting both `DOUBLE_GYRE_DEFAULT_SETTINGS` and `PERLIN_DEFAULT_SETTINGS` to `0` will default in an alternative double gyre physics preset.

Setting both `USE_GPU` and `USE_CPU_PARALLELISM` to `0` will default in using a sequetial CPU implementation.



