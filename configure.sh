#!/bin/bash

# Default value for ABI if not provided
ABI=${1:-arm64-v8a}
NDK=${2:-/home/martin/Android/Sdk/ndk/25.1.8937393}

# Script operations using the ABI variable
echo "Using ABI: $ABI"

# Get third party libs
cd simulation/third_party

# Get and install zlib
mkdir zlib
cd zlib
mkdir source
mkdir build
cd source
curl -L -o zlib.tar.gz https://github.com/madler/zlib/releases/download/v1.3.1/zlib-1.3.1.tar.gz
tar -xzf zlib.tar.gz --strip-components=1
cd ../../build_scripts
chmod +x compile_zlib.sh
./compile_zlib.sh $ABI $NDK
cd ..

# Get and install hdf5
mkdir hdf5
cd hdf5
mkdir source
mkdir build
cd source
curl -L -o hdf5.tar.gz https://github.com/HDFGroup/hdf5/archive/refs/tags/hdf5_1.14.4.2.tar.gz
tar -xzf hdf5.tar.gz --strip-components=1
cd ../../build_scripts
chmod +x compile_hdf5.sh
./compile_hdf5.sh $ABI $NDK
cd ..

# Get and install netcdf
mkdir netcdf-c
cd netcdf-c
mkdir source
mkdir build
cd source
curl -L -o netcdf-c.tar.gz https://github.com/Unidata/netcdf-c/archive/refs/tags/v4.9.2.tar.gz
tar -xzf netcdf-c.tar.gz --strip-components=1
cd ../../build_scripts
chmod +x compile_netcdf.sh
./compile_netcdf.sh $ABI $NDK
cd ..

# Get and install netcdf-cxx
mkdir netcdf-cxx
cd netcdf-cxx
mkdir source
mkdir build
cd source
curl -L -o netcdf-cxx.tar.gz https://github.com/Unidata/netcdf-cxx4/archive/refs/tags/v4.3.1.tar.gz
tar -xzf netcdf-cxx.tar.gz --strip-components=1
cd ../../build_scripts
chmod +x compile_netcdf_cxx.sh
./compile_netcdf_cxx.sh $ABI $NDK
cd ..

# Get and install vtk
mkdir vtk
cd vtk
mkdir source
mkdir build
cd source
curl -L -o vtk.tar.gz https://www.vtk.org/files/release/9.3/VTK-9.3.0.tar.gz
tar -xzf vtk.tar.gz --strip-components=1
cd ../../build_scripts
chmod +x compile_vtk.sh
./compile_vtk.sh $ABI $NDK
cd ..

# Copy shared libraries to the app
cd ..
mkdir -p app/src/main/jniLibs/$ABI
cp -r $NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/$ABI/libc++_shared.so app/src/main/jniLibs/$ABI
cp -r $NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/$ABI/libhdf5.so app/src/main/jniLibs/
cp -r $NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/$ABI/libhdf5_hl.so app/src/main/jniLibs/
cp -r $NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/$ABI/libhdf5_cpp.so app/src/main/jniLibs/
cp -r $NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/$ABI/libnetcdf.so app/src/main/jniLibs/
cp -r $NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/$ABI/libnetcdf_c++.so app/src/main/jniLibs/
cp -r $NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/$ABI/libz.so app/src/main/jniLibs/

# Make dependencies in shared libraries be compatible with the built ndk
cd app/src/main/jniLibs/$ABI
patchelf --set-soname libz.so libz.so
patchelf --replace-needed libz.so.1 libz.so libhdf5.so
patchelf --replace-needed libz.so.1 libz.so libhdf5_hl.so
patchelf --replace-needed libz.so.1 libz.so libhdf5_cpp.so
patchelf --replace-needed libz.so.1 libz.so libnetcdf.so
patchelf --replace-needed libz.so.1 libz.so libnetcdf_c++.so
cd ../../../../..


# Copy static libraries to the app
mkdir -p app/src/main/cpp/include
mkdir -p app/src/main/cpp/lib/$ABI
cp -r third_party/vtk/build/CMakeExternals/Install/vtk-android/include/* app/src/main/cpp/include
cp -r third_party/vtk/build/CMakeExternals/Install/vtk-android/lib/$ABI/* app/src/main/cpp/lib/$ABI
