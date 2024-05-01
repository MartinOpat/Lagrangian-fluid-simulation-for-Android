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

# Set Position Independent Code to ON for all targets
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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


add_library(zlib SHARED IMPORTED)
set_target_properties(zlib PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libz.so)


add_library(hdf5 SHARED IMPORTED)
set_target_properties(hdf5 PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libhdf5.so)

add_library(netcdf SHARED IMPORTED)
set_target_properties(netcdf PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libnetcdf.so)

add_library(netcdf_cxx SHARED IMPORTED)
set_target_properties(netcdf_cxx PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libnetcdf_c++4.so)

add_library(libc++_shared SHARED IMPORTED)
set_target_properties(libc++_shared PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libc++_shared.so)

add_library(${CMAKE_PROJECT_NAME} SHARED
        native-lib.cpp
        png_loader.cpp
        mainview.cpp
        netcdf_reader.cpp
        particle.cpp
        triple.cpp
)

# Static libraries headers
include_directories(${CMAKE_SOURCE_DIR}/include/vtk-9.3/)

# Add the VTK static libraries
file(GLOB VTK_LIBRARIES "${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}/*.a")

# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(${CMAKE_PROJECT_NAME}
        # List libraries link to the target library
        android
        log
        GLESv3
        netcdf_cxx
        netcdf
        ${VTK_LIBRARIES}
)


# For including libraries (outside NDK) later on
# include_directories(include/)

# TODO: Maybe add this to make sure the C++ standard is enforced
#set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
#        CXX_STANDARD 17
#        CXX_STANDARD_REQUIRED ON)

# Compiler flags
# target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra -O2)

