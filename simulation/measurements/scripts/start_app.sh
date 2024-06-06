#!/bin/bash

#./gradlew assembleDebug
#adb install -r ./app/build/outputs/apk/debug/app-debug.apk
./gradlew assembleRelease

# Check if the build was successful
if [ ! -f "$APK_PATH" ]; then
    echo "Build failed, APK not found."
    exit 1
fi

# Install and start
adb install -r ./app/build/outputs/apk/release/app-release.apk
adb shell am start -n com.rug.lagrangianfluidsimulation/.MainActivity


# Wait for the app to load
sleep 10

# First tap (coordinates: 319, 962)
adb shell input tap 319 962
echo "First tap executed."
sleep 3

# Second tap (coordinates: 576, 2128)
adb shell input tap 576 2128
echo "Second tap executed."
sleep 3

# Third tap (coordinates: 899, 2078)
adb shell input tap 899 2078
echo "Third tap executed."