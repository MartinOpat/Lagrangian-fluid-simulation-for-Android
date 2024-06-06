#!/bin/bash

# Define the log file location
LOGFILE="logs.txt"

# Check if adb is connected
adb devices | grep -q device
if [ $? -eq 0 ]; then
    echo "Device is connected. Starting to capture logs..."
else
    echo "No device connected. Please connect your device and try again."
    exit 1
fi

# Clear the current logs
adb logcat -c

# Start capturing logs. Adjust the grep pattern to match how the fps is logged.
adb logcat | grep "FPS" >> $LOGFILE

