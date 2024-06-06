#!/bin/bash

NUM_PARTICLES=$1
MODE=$2

./gradlew clean

#./gradlew assembleDebug
./gradlew assembleRelease

# Install and start
#adb install -r ./app/build/outputs/apk/debug/app-debug.apk
adb install -r ./app/build/outputs/apk/release/app-release.apk
adb shell am start -n com.rug.lagrangianfluidsimulation/.MainActivity \
  --es "NUM_PARTICLES" $NUM_PARTICLES \
  --es "MODE" $MODE


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

