#!/bin/bash

ABI=${1:-arm64-v8a}
NDK=${2:-/home/martin/Android/Sdk/ndk/25.1.8937393}


#export NDK=/home/martin/Android/Sdk/ndk/25.1.8937393
export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64
export TARGET=$ABI-linux-android
export API=21

# Setup Compiler Variables
export AR=$TOOLCHAIN/bin/llvm-ar
export CC=$TOOLCHAIN/bin/$TARGET$API-clang
export CXX=$TOOLCHAIN/bin/$TARGET$API-clang++
export LD=$TOOLCHAIN/bin/ld
export RANLIB=$TOOLCHAIN/bin/llvm-ranlib
export STRIP=$TOOLCHAIN/bin/llvm-strip

# Setup Additional Flags
export SYSROOT=$TOOLCHAIN/sysroot
export CFLAGS="--sysroot $SYSROOT"
export CXXFLAGS="--sysroot $SYSROOT"
export LDFLAGS="--sysroot $SYSROOT -L$SYSROOT/usr/lib"
export CPPFLAGS="-I$SYSROOT/usr/include"

# Ensure the netcdf C library paths are correctly set
export NETCDF_C_ROOT=$SYSROOT/usr

# Change to the netcdf-cxx directory
cd ../netcdf-cxx/source

./configure --host=$TARGET --prefix=$SYSROOT/usr --with-netcdf=$NETCDF_C_ROOT --disable-dap --enable-shared --disable-static
make && make install
cd ../../build_scripts
