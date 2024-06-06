#!/bin/bash

BASE_LOGFILE="logs.txt"

# Find the next available log file name
counter=0
LOGFILE="${BASE_LOGFILE%.txt}$counter.txt"
while [[ -f $LOGFILE ]]; do
    let counter+=1
    LOGFILE="${BASE_LOGFILE%.txt}$counter.txt"
done

echo "Logging to $LOGFILE"

# Function to handle script termination
cleanup() {
    echo "Caught interrupt signal. Cleaning up..."
    sync
    exit 0
}
trap cleanup SIGINT

# Check if adb is connected
if ! adb devices | grep -q device; then
    echo "No device connected. Please connect your device and try again."
    exit 1
fi
echo "Device is connected. Starting to capture logs..."

# Clear the current logs
adb logcat -c

# Start capturing logs
adb logcat | grep -E 'Timer\s*: Elapsed time: [0-9]+\.[0-9]+' >> $LOGFILE &
ADB_PID=$!

# Wait indefinitely until the script is killed
wait $ADB_PID
