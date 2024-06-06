#!/bin/bash

# Define configurations
#declare -a modes=("computeShaders" "sequential" "parallel")
declare -a modes=("sequential" "computeShaders")
#declare -a particleCounts=(1500 3500 7500 17000 37000 85000 190000 420000 950000 2200000 5000000)
declare -a particleCounts=(10 10000)



# Loop over configurations
for mode in "${modes[@]}"; do
    for num in "${particleCounts[@]}"; do
        echo "Setting NUM_PARTICLES to $num and Mode to $mode"

        # Set environment variables (if your app can read these in production)
        export NUM_PARTICLES=$num
        export MODE=$mode

        # Build the production APK
        ./measurements/scripts/start_app.sh $num $mode

        # Measure the logs
        ./measurements/scripts/capture_logs.sh $num $mode

        # Stop the app
        adb shell am force-stop com.rug.lagrangianfluidsimulation
    done
done
