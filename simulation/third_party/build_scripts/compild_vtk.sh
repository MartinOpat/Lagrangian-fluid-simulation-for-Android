#!/bin/bash

# Define NDK path and toolchain
export NDK=/home/martin/Android/Sdk/ndk/25.1.8937393
export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64
export TARGET=aarch64-linux-android
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
export CFLAGS="--sysroot $SYSROOT -fpic"
export CXXFLAGS="--sysroot $SYSROOT -std=c++17 -fpic -fexceptions -frtti"
export LDFLAGS="--sysroot $SYSROOT -L$SYSROOT/usr/lib"
export CPPFLAGS="-I$SYSROOT/usr/include"

# Define cmake toolchain file for Android
export CMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake
export ANDROID_NDK=$NDK

# Change to the VTK directory
cd ../VTK-9.3.0
mkdir build
cd build

# Run CMake to configure the VTK build
cmake -DCMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_FILE \
      -DCMAKE_ANDROID_NDK=$NDK \
      -DCMAKE_VERBOSE_MAKEFILE=ON \
      -DCMAKE_SYSTEM_NAME=Android \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_ANDROID_STL_TYPE=c++_static \
      -DCMAKE_SYSTEM_VERSION=$API \
      -DVTK_ANDROID_BUILD=ON \
      -DANDROID_NATIVE_API_LEVEL=$API \
      -DOPENGL_ES_VERSION=3.0 \
      -DANDROID_ABI=arm64-v8a \
      ..

# Build VTK
cmake --build .
