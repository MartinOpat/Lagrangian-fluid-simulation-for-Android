#!/bin/bash

NUM_PARTICLES=$1
MODE=$2

cd measurements/files

# Construct log file name based on the extracted values
BASE_LOGFILE="logs_${NUM_PARTICLES}_${MODE}.txt"
counter=0
LOGFILE="${BASE_LOGFILE%.txt}$counter.txt"
while [[ -f $LOGFILE ]]; do
    let counter+=1
    LOGFILE="${BASE_LOGFILE%.txt}$counter.txt"
done
echo "Logging to $LOGFILE"


# Check if adb is connected
if ! adb devices | grep -q device; then
    echo "No device connected. Please connect your device and try again."
    exit 1
fi
echo "Device is connected. Starting to capture logs..."

# Clear the current logs and start capturing logs
set -m
adb logcat -c
#adb logcat | grep -E 'Timer\s*: Elapsed time: [0-9]+\.[0-9]+' >> $LOGFILE &
adb logcat | grep -E '\b[A-Za-z]*Timer[A-Za-z]*\s*:\s*Elapsed time: [0-9]+\.[0-9]+' >> $LOGFILE &
ADB_PID=$!
set +m

echo "Logging for 5 minutes..."
sleep 300

echo "Time is up, killing logging process..."

kill -- -$ADB_PID
echo "Logs captured and saved to $LOGFILE"
sleep 5  # I am trying to be nice here
if kill -0 $ADB_PID 2>/dev/null; then
    echo "Process did not terminate, sending SIGKILL..."
    kill -9 $ADB_PID
fi
wait $ADB_PID 2>/dev/null

echo "exiting..."
cd ../..