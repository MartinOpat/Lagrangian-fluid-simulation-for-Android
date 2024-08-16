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
set(CMAKE_CXX_STANDARD 17)

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

add_subdirectory(glm)
include_directories(netcdf)

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
        src/native-lib.cpp
        src/mainview.cpp
        src/file_reader.cpp
        src/netcdf_reader.cpp
        src/vector_field_handler.cpp
        src/particle.cpp
        src/particles_handler.cpp
        src/touch_handler.cpp
        src/physics.cpp
        src/transforms.cpp
        src/EGLContextManager.cpp
        src/shaderManager.cpp
)


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
        glm
        EGL
)

# Config file exporting environment variables
set(CONFIG_FILE "${CMAKE_SOURCE_DIR}/config.txt")

# Load config file func.
function(load_config FILE_PATH)
    file(STRINGS ${FILE_PATH} ConfigLines REGEX "^[^#]+=.+")
    foreach(Line ${ConfigLines})
        string(REGEX REPLACE "^[ ]*([^= ]+)[ ]*=.*" "\\1" Key ${Line})
        string(REGEX REPLACE "^[^=]*=[ ]*(.*)" "\\1" Value ${Line})
        set(${Key} "${Value}" CACHE INTERNAL "Config value")
        message("Configured ${Key} as ${Value}")
    endforeach()
endfunction()
load_config(${CONFIG_FILE})

# Add definitions for C++
if(LOAD_POSITIONS_FROM_FILE)
    add_definitions(-DLOAD_POSITIONS_FROM_FILE=${LOAD_POSITIONS_FROM_FILE})
endif()
if (REDUCE_FIELD_GRAPHICS)
    add_definitions(-DREDUCE_FIELD_GRAPHICS=${REDUCE_FIELD_GRAPHICS})
endif()
if (DOUBLE_GYRE_DEFAULT_SETTINGS)
    add_definitions(-DDOUBLE_GYRE_DEFAULT_SETTINGS=${DOUBLE_GYRE_DEFAULT_SETTINGS})
endif()
if (PERLIN_DEFAULT_SETTINGS)
    add_definitions(-DPERLIN_DEFAULT_SETTINGS=${PERLIN_DEFAULT_SETTINGS})
endif()
if (USE_GPU)
    add_definitions(-DUSE_GPU=${USE_GPU})
endif()
if (USE_CPU_PARALLELISM)
    add_definitions(-DUSE_CPU_PARALLELISM=${USE_CPU_PARALLELISM})
endif()

# For including libraries (outside NDK) later on
# include_directories(include/)

# Compiler flags
# target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra -O2)

