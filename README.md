# Lagrangian-fluid-simulation-for-Android

## Table of contents
1. [Build instructions](#build-instructions)
    1. [SDK and NDK installation](#sdk-and-ndk-installation)
    2. [Building the app](#building-the-app)
2. [Installing the app](#installing-the-app)



## Build instructions
### SDK and NDK installation
To build the application, Android SDK and NDK are required on the system. The best way to get the SDK and NDK is via [AndroidStudio](https://developer.android.com/studio), as this ensures compatibility with the project structure.

The SDK and NDK can be installed either at the initial (one-time) setup for AndroidStudio or via `File -> Settings -> Android SDK -> SDK tools`. There, make sure (at least) the following list is selected:
- Android SDK Build-tools
- NDK (side by side)
- Android SDK Platform-tools

After pressing `apply`, an installer should appear. Once everything is finished, you can leave the settings.

Note that SDK `34.0.0` and NDK `25.1.8937393` were used during the development.

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