#!/bin/bash

# Define configurations
#declare -a modes=("sequential" "parallel" "computeShaders")
declare -a modes=("parallel")
#declare -a modes=("computeShaders")
#declare -a modes=("sequential" "computeShaders")
#declare -a particleCounts=(1500 3500 7500 17000 37000 85000 190000 420000 950000 2200000 5000000)
declare -a particleCounts=(1500 3500 7500 17000 37000 85000 190000)
#declare -a particleCounts=(5000000 2200000 950000 420000 190000 85000 37000 17000 7500 3500 1500)
#declare -a particleCounts=(5000000)
#declare -a particleCounts=(250 250000)
#declare -a particleCounts=(1 5 25 125 625)
#declare -a particleCounts=(3500)

declare -a coresCounts=(8)  # This is obsolete, needs just 2 and 4 so changing it manually



# Loop over configurations
for mode in "${modes[@]}"; do
    for num in "${particleCounts[@]}"; do
        for core in "${coresCounts[@]}"; do
            echo "Setting NUM_PARTICLES to $num and Mode to $mode and cores to $core"

            # Set environment variables (if your app can read these in production)
            export NUM_PARTICLES=$num
            export MODE=$mode
            export CORES=$core

            # Build the production APK
            ./measurements/scripts/start_app.sh $num $mode

            # Measure the logs
            ./measurements/scripts/capture_logs.sh $num $mode

            # Stop the app
            adb shell am force-stop com.rug.lagrangianfluidsimulation
        done
    done
done
