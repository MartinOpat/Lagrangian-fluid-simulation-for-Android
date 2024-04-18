export NDK=/home/martin/Android/Sdk/ndk/25.1.8937393
export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64
export TARGET=x86_64-linux-android
export API=21

# Setup Compiler Variables
export AR=$TOOLCHAIN/bin/llvm-ar
export CC=$TOOLCHAIN/bin/$TARGET$API-clang
export CXX=$TOOLCHAIN/bin/$TARGET$API-clang++
export LD=$TOOLCHAIN/bin/ld
export RANLIB=$TOOLCHAIN/bin/llvm-ranlib
export STRIP=$TOOLCHAIN/bin/llvm-strip

# Setup Additional Flags
export SYSROOT=$TOOLCHAIN/sysrootxrea
export CFLAGS="--sysroot $SYSROOT"
export CXXFLAGS="--sysroot $SYSROOT"
export LDFLAGS="--sysroot $SYSROOT -L$SYSROOT/usr/lib"
export CPPFLAGS="-I$SYSROOT/usr/include"


cd ../hdf5-hdf5_1.14.4.2
./configure --host=$TARGET --prefix=$SYSROOT/usr --with-zlib=$SYSROOT/usr --enable-shared --disable-static
make && make install
