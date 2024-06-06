#!/bin/bash

# Extract NUM_PARTICLES and mode from native files
SOURCE_FILE="app/src/main/cpp/src/native-lib.cpp"
NUM_PARTICLES=$(awk '/#define NUM_PARTICLES/ {print $3}' "$HEADER_FILE")
MODE=$(awk -F'::' '/mode = Mode::/ {print $2; exit}' "$SOURCE_FILE" | tr -d ' ;')


# Construct log file name based on the extracted values
BASE_LOGFILE="logs_${NUM_PARTICLES}_${MODE}.txt"
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

# Clear the current logs and start capturing logs
adb logcat -c
adb logcat | grep -E 'Timer\s*: Elapsed time: [0-9]+\.[0-9]+' >> $LOGFILE &
ADB_PID=$!

# Wait indefinitely until the script is killed
wait $ADB_PID
