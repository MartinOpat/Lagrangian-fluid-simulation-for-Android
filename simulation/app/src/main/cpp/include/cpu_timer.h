//
// Created by martin on 12-06-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_CPU_TIMER_H
#define LAGRANGIAN_FLUID_SIMULATION_CPU_TIMER_H

#include "android_logging.h"

#include <algorithm>
#include <chrono>

class CpuTimer {
public:
    CpuTimer();
    void start();
    void stop();
    double elapsed_milliseconds();
    void logElapsedTime();
    void measure();


private:
    bool started;
    std::clock_t startTime, stopTime;
    int numMeasurements;
    std::chrono::time_point<std::chrono::steady_clock> startWallTime, stopWallTime;
    std::chrono::milliseconds displayFrequency;
};

inline CpuTimer::CpuTimer()
        : started(false), numMeasurements(0), displayFrequency(std::chrono::milliseconds(1000)) {}

inline void CpuTimer::start() {
    started = true;
    numMeasurements = 0;
    startTime = std::clock();
}

inline void CpuTimer::stop() {
    started = false;
    stopTime = std::clock();
}

inline double CpuTimer::elapsed_milliseconds() {
    double t = (double)(std::clock() - startTime) * 1000.0 / CLOCKS_PER_SEC;
//    LOGI("CpuTimer", "StartTime: %ld", startTime);
//    LOGI("CpuTimer", "Elapsed time: %f ms", t);
    return t;
}

inline void CpuTimer::logElapsedTime() {
    LOGI("CpuTimer", "Elapsed time: %f ms", elapsed_milliseconds() / numMeasurements);
}

inline void CpuTimer::measure() {
    if (started) {
        numMeasurements++;
        auto elapsedWallTime = std::chrono::steady_clock::now() - startWallTime;
        if (std::chrono::duration_cast<std::chrono::milliseconds>(elapsedWallTime) >= displayFrequency) {
            stop();
            logElapsedTime();
            start();
            startWallTime = std::chrono::steady_clock::now();
        }
    } else {
        start();
    }
}

#endif //LAGRANGIAN_FLUID_SIMULATION_CPU_TIMER_H
