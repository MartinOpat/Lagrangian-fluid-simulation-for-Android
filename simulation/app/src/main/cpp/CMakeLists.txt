# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

# Declares the project name. The project name can be accessed via ${ PROJECT_NAME},
# Since this is the top level CMakeLists.txt, the project name is also accessible
# with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level
# build script scope).
project("lagrangianfluidsimulation")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#
# In this top level CMakeLists.txt, ${CMAKE_PROJECT_NAME} is used to define
# the target library name; in the sub-module's CMakeLists.txt, ${PROJECT_NAME}
# is preferred for the same purpose.
#
# In order to load a library into your app from Java/Kotlin, you must call
# System.loadLibrary() and pass the name of the library defined here;
# for GameActivity/NativeActivity derived applications, the same library name must be
# used in the AndroidManifest.xml file.


# Subdirectories to include
#include_directories(
#        ${CMAKE_SOURCE_DIR}/hdf5-hdf5_1.14.4.2/src
#        ${CMAKE_SOURCE_DIR}/hdf5-hdf5_1.14.4.2/c++/src
#        ${CMAKE_SOURCE_DIR}/../../../../third_party/hdf5/include
#        ${CMAKE_SOURCE_DIR}/netcdf-c-4.9.2/include
#        ${CMAKE_SOURCE_DIR}/netcdf-cxx4-4.3.1/cxx4
#        libs/netcdfpp-main/include
#)

include_directories(${ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include)
add_library(netcdf STATIC IMPORTED)
set_target_properties(netcdf PROPERTIES IMPORTED_LOCATION ${ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/libnetcdf.a)
#
## Assuming the dependencies are in the same lib path
#add_library(hdf5 STATIC IMPORTED)
#set_target_properties(hdf5 PROPERTIES IMPORTED_LOCATION ${ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/libhdf5.a)
#
#add_library(zlib STATIC IMPORTED)
#set_target_properties(zlib PROPERTIES IMPORTED_LOCATION ${ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/libz.a)
#
## Link the libraries
#target_link_libraries(netcdf hdf5 zlib)



# Automatically add all source files in the specified directory
#file(GLOB_RECURSE PROJECT_SOURCES "${CMAKE_SOURCE_DIR}/*.cpp")
#file(GLOB_RECURSE HDF5_SOURCES "${CMAKE_SOURCE_DIR}/hdf5-hdf5_1.14.4.2/c++/src/*.cpp")
#file(GLOB_RECURSE HDF5_CXX_SOURCES "${CMAKE_SOURCE_DIR}/hdf5-hdf5_1.14.4.2/c++/src/*.cpp")
#file(GLOB_RECURSE NETCDF_SOURCES "${CMAKE_SOURCE_DIR}/netcdf-c-4.9.2/libsrc/*.c")
#file(GLOB_RECURSE NETCDF_CXX_SOURCES "${CMAKE_SOURCE_DIR}/netcdf-cxx4-4.3.1/cxx4/*.cpp")

# Old manual setup
#add_library(${CMAKE_PROJECT_NAME} SHARED
#        native-lib.cpp
#        png_loader.cpp
#        mainview.cpp
#)

#add_library(hdf5 SHARED IMPORTED)
#set_target_properties(hdf5 PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../../../../third_party/hdf5/lib/libhdf5.so)

add_library(${CMAKE_PROJECT_NAME} SHARED
        native-lib.cpp
        png_loader.cpp
        mainview.cpp
#        ${HDF5_SOURCES}
#        ${HDF5_CXX_SOURCES}
#        ${NETCDF_SOURCES}
#        ${NETCDF_CXX_SOURCES}
)

#add_subdirectory(libs/zlib-1.3.1)
#add_subdirectory(libs/hdf5-hdf5_1.14.4.2)
#add_subdirectory(libs/netcdf-c-4.9.2)
#add_subdirectory(libs/netcdfpp-main)
#add_subdirectory(libs/netcdf-cxx4-4.3.1)
#add_subdirectory(libs/ncpp-master)


# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(${CMAKE_PROJECT_NAME}
        # List libraries link to the target library
        android
        log
        GLESv2
#        hdf5
        netcdf
#        netcdf-cxx4
)


# For including libraries (outside NDK) later on
# include_directories(include/)

# TODO: Maybe add this to make sure the C++ standard is enforced
#set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
#        CXX_STANDARD 17
#        CXX_STANDARD_REQUIRED ON)

# Compiler flags
# target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra -O2)

