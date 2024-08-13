# Lagrangian-fluid-simulation-for-Android

## Table of contents
1. [Build instructions](#build-instructions)
    1. [SDK and NDK installation](#sdk-and-ndk-installation)
    2. [Building the app](#building-the-app)
2. [Installing the app](#installing-the-app)
3. [3rd party libraries](#3rd-party-libraries)



## Build instructions
### SDK and NDK installation
To build the application, Android SDK and NDK are required on the system. The best way to get the SDK and NDK is via [AndroidStudio](https://developer.android.com/studio), as this ensures compatibility with the project structure.

The SDK and NDK can be installed either at the initial (one-time) setup for AndroidStudio or via `File -> Settings -> Android SDK -> SDK tools`. There, make sure (at least) the following list is selected:
- Android SDK Build-tools
- NDK (side by side)
- Android SDK Platform-tools

After pressing `apply`, an installer should appear. Once everything is finished, you can leave the settings. Also make sure that Java 17+ is installed, if not you can install it via:
```bash
sudo apt-get install openjdk-17-jdk
```

Note that SDK `34.0.0` and NDK `25.1.8937393` were used during the development (see [3rd party applications](#3rd-party-applications) for more detailed info)

### Building the app
The app can be built via [AndroidStudio](https://developer.android.com/studio) or by running
```bash
./gradlew clean
./gradlew assembleDebug
```
inside the `simulation` folder. The `apk` can then be found at `simulation/app/build/outputs/apk/debug/app-debug.apk`. For a release version, you can run:
```bash
./gradlew assembleRelease
```
However, a valid `.jks` key has to first be generated. The `apk` will then be at `simulation/app/build/outputs/apk/release/app-release.apk`

## Installing the app
First things first, make sure your Android device is connected. USB debugging from Developers options is recommended to be turned on.

If you connect your phone to [AndroidStudio](https://developer.android.com/studio), you can install the `apk` simply by dragging it onto the screen. Alternatively, you can run 
```bash
adb install -r ./app/build/outputs/apk/debug/app-debug.apk
```
from the `simulation` folder to install the `apk`.


## 3rd Party Libraries
The following list of 3rd party applications was used in this project:
- [zlib](https://github.com/madler/zlib) 1.3.1
- [hdf5](https://github.com/HDFGroup/hdf5) 1.14.4.2
- [netcdf-c](https://github.com/Unidata/netcdf-c) 4.9.2
- [netcdf-cxx](https://github.com/Unidata/netcdf-cxx4) 4.3.1

The libraries above have already been compiled for Android into shader objects (.so) and included in the project. Thus, there is no need to recompile them. However, follow the `simulation/third_party/` instructions if you need to recompile them anyway.

 The following 3rd party C++ libraries were used in the native implementation:
- [GLM](https://github.com/g-truc/glm) 1.0.1
- [ThreadPool](https://github.com/progschj/ThreadPool) 

For compiling and building the project for Android, the following list of tools was used:
- [NDK](https://developer.android.com/ndk) 25.1.8937393
- [Android SDK Build-Tools](https://developer.android.com/tools/releases/build-tools) 34.0.0
- [Android SDK Platform-Tools](https://developer.android.com/tools/releases/platform-tools) 35.0.1
- [CMake](https://cmake.org/) 3.22.1
- [Gradle](https://gradle.org/) 8.4
