#!/bin/bash

# ABI=${1:-arm64-v8a}
# NDK=${2:-/home/martin/Android/Sdk/ndk/25.1.8937393}
echo "Using ABI: ${ABI}"
echo "Using NDK location: ${NDK}"
echo "Using architecture: ${ARCH} (compiler prefix: ${COMPILER_PREFIX})"

# Define NDK path and toolchain
export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64
# export TARGET=$ABI-linux-android
export TARGET=$COMPILER_PREFIX
# export API=29
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
ccmake -DVTK_ANDROID_BUILD=ON \
       -DANDROID_NDK=$NDK \
       -DANDROID_NATIVE_API_LEVEL=$API \
       -DANDROID_ARCH_ABI=$ABI \
       ../source

# Build VTK
#cmake --build .
#cmake --install .
#cd ../../build_scripts
