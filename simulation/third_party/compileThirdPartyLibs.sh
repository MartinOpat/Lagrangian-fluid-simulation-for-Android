#!/bin/bash

# Default value for ABI if not provided
export ABI=${1:-arm64-v8a}
export NDK=${2:-/home/martin/Android/Sdk/ndk/25.1.8937393}

# Script operations using the ABI variable
echo "Using ABI: $ABI"
echo "Using NDK location: $NDK"

# architecture scanning
export ARCH=""
export COMPILER_PREFIX=""

case $ABI in
    "arm64-v8a"|"arm64v8a"|"arm64_v8a")
	export ARCH="aarch64"
	export COMPILER_PREFIX="${ARCH}-linux-android";;
    "armeabi-v7a"|"arm-v7a"|"armv7a")
	export ARCH="armv7a"
	export COMPILER_PREFIX="${ARCH}-linux-androideabi";;
    "x86")
	export ARCH="i686"
	export COMPILER_PREFIX="${ARCH}-linux-android";;
    "x86_64")
	export ARCH="armv7a"
	export COMPILER_PREFIX="${ARCH}-linux-android";;
    *) ;;
esac
echo "Using architecture: ${ARCH} (compiler prefix: ${COMPILER_PREFIX})"

# Get third party libs

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

# Copy shared libraries to the app
cd ..
mkdir -p app/src/main/jniLibs/$ABI
cp -r $NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/$COMPILER_PREFIX/libc++_shared.so app/src/main/jniLibs/$ABI
cp -r $NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/libhdf5.so app/src/main/jniLibs/$ABI
cp -r $NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/libhdf5_hl.so app/src/main/jniLibs/$ABI
cp -r $NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/libhdf5_cpp.so app/src/main/jniLibs/$ABI
cp -r $NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/libnetcdf.so app/src/main/jniLibs/$ABI
cp -r $NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/libnetcdf_c++4.so app/src/main/jniLibs/$ABI
cp -r $NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/libz.so.1.3.1 app/src/main/jniLibs/$ABI
mv app/src/main/jniLibs/$ABI/libz.so.1.3.1 app/src/main/jniLibs/$ABI/libz.so

# Make dependencies in shared libraries be compatible with the built ndk
cd app/src/main/jniLibs/$ABI
patchelf --set-soname libz.so libz.so
patchelf --replace-needed libz.so.1 libz.so libhdf5.so
patchelf --replace-needed libz.so.1 libz.so libhdf5_hl.so
patchelf --replace-needed libz.so.1 libz.so libhdf5_cpp.so
patchelf --replace-needed libz.so.1 libz.so libnetcdf.so
patchelf --replace-needed libz.so.1 libz.so libnetcdf_c++4.so
cd ../../../../..
