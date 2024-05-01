#!/bin/bash

# Define NDK path and toolchain
export NDK=/home/martin/Android/Sdk/ndk/25.1.8937393
export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64
export ABI=arm64-v8a
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

# Define cmake toolchain file for Android
export CMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake
export ANDROID_NDK=$NDK

# Change to the VTK directory
cd ../vtk/build


# Configure VTK using CMake for Android
cmake -DVTK_ANDROID_BUILD=ON \
       -DANDROID_NDK=$NDK \
       -DANDROID_NATIVE_API_LEVEL=$API \
       -DANDROID_ARCH_ABI=$ABI \
       ../source

# Build VTK
cmake --build .

# Install VTK
cmake --install .