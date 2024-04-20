# How to install the necessary third-party libraries

Firstly, make sure all the necessary packages are installed on your system. On Ubuntu, you can run:
```bash
sudo apt-get install build-essential
sudo apt-get install cmake
sudo apt-get install libxml2-dev
````
Note that other packages might be necessary. In case on of these libraries is missing, the error message will usually tell you which package is missing.

Make sure that for all required third-party libraries (currently [zlib](https://github.com/madler/zlib), [hdf5](https://github.com/HDFGroup/hdf5) and [netcdf](https://github.com/Unidata/netcdf-c)) you download the source files and place (extract) them in the `third_party` directory.
Assuming you are in the `third_party/build_scripts` directory, to install [zlib](https://github.com/madler/zlib) you can run:
```bash
sudo bash compile_zlib.sh
```

Similarly, to install [hdf5](https://github.com/HDFGroup/hdf5) you can run:
```bash
sudo bash compile_hdf5.sh
```

To install [netcdf](https://github.com/Unidata/netcdf-c) you can run:
```bash
sudo bash compile_netcdf.sh
```

Lastly, to install [netcdf-cxx4](https://github.com/Unidata/netcdf-cxx4) you can run:
```bash
sudo bash compile_netcdf-cxx4.sh
```

Note that the path to the NDK used by the project (inside the `.sh` files) might need to be updated to match the path to the NDK on your system. Similarly, other flags can be adjusted to match the desired configuration.
Make sure to copy all the newly build `.so` files to the `app/src/main/jniLibs` directory. Sometimes it happens that the libraries `libhdf5.so`, `libhdf5_hl.so` and `libnetcdf.so` have a dependency on `libz.so.1` which cannot be automatically added into the the apk due to android NDK's internal regex. You can check this by running `readelf -d libhdf5.so` and looking for `libz.so.1`. If you see it, you can change it into `libz.so` by running `patchelf --replace-needed libz.so.1 libz.so libhdf5.so`. This will make the library compatible with the android NDK.
