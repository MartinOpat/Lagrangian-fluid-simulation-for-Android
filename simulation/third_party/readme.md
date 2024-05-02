# How to install the necessary third-party libraries manually
Note that it is recommended to use the provided configure bash script in the root of this repo.

## Installing prerequisites
Firstly, make sure all the necessary packages are installed on your system. On Ubuntu, you can run:
```bash
sudo apt-get install build-essential
sudo apt-get install cmake
sudo apt-get install libxml2-dev
sudo apt-get install curl
sudo apt-get install patchelf
````
Note that other packages might be necessary. In case on of these libraries is missing, the error message will usually tell you which package is missing.

## Shared libraries
Make sure that for all required third-party libraries (currently [zlib](https://github.com/madler/zlib), [hdf5](https://github.com/HDFGroup/hdf5) and [netcdf](https://github.com/Unidata/netcdf-c)) you download the source files and place (extract) them in the `third_party` directory.

### Installing zlib
Assuming you are in the `third_party/build_scripts` directory, to install [zlib](https://github.com/madler/zlib) you can run:
```bash
sudo bash compile_zlib.sh
```

### Installing hdf5
Similarly, to install [hdf5](https://github.com/HDFGroup/hdf5) you can run:
```bash
sudo bash compile_hdf5.sh
```

### Installing netcdf
To install [netcdf](https://github.com/Unidata/netcdf-c) you can run:
```bash
sudo bash compile_netcdf.sh
```

### Installing netcdf-cxx
Lastly, to install [netcdf-cxx4](https://github.com/Unidata/netcdf-cxx4) you can run:
```bash
sudo bash compile_netcdf-cxx4.sh
```

Note that the path to the NDK used by the project (inside the `.sh` files) might need to be updated to match the path to the NDK on your system. All the above bash script take two optional arguments, the ABI and the NDK to easily match the needed configuration.
Other flags (e.g. compiler variables) have to be adjusted inside the bash scripts directly.

### Using the built libraries in the android project
Make sure to copy all the necessary newly build shared libraries (i.e. `.so` files) to the `app/src/main/jniLibs` directory. Sometimes it happens that the libraries `libhdf5.so`, `libhdf5_hl.so` and `libnetcdf.so` have a dependency on `libz.so.1` which cannot be automatically added into the the apk due to android NDK's internal regex. You can check this by running `readelf -d libhdf5.so` and looking for `libz.so.1`. If you see it, you can change it into `libz.so` by running `patchelf --replace-needed libz.so.1 libz.so libhdf5.so`. This will make the library compatible with the android NDK. Note that the recommended configure script in the root of this repo will do this automatically.

## Static libraries
To use this project, a custom build of [vtk](https://vtk.org/download/) is necessary. The build script is located in the `third_party/build_scripts` directory. To build the static libraries, you can run:
```bash
sudo bash compile_vtk.sh
```
The structure of the bash compile file is the same as for the shared libraries (see prev. section). After successfully building the static libraries, you can copy the built static libraries to the `app/src/main/cpp/libs` directory. Similarly, the include (header) files should be copied to the `app/src/main/cpp/include` directory. The built files can be found in the `third_party/vtk/build/CMakeExternals/Install/vtk-android` directory.