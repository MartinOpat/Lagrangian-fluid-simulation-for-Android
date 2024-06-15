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
    void logElapsedTime(std::string tag="");
    void measure();
    void countMeasurement();
    void reset();


private:
    bool started;
    std::clock_t startTime, stopTime;
    int numMeasurements;
    double measurements;
    std::chrono::time_point<std::chrono::steady_clock> startWallTime, stopWallTime;
    std::chrono::milliseconds displayFrequency;
};

inline CpuTimer::CpuTimer()
        : started(false), numMeasurements(0), displayFrequency(std::chrono::milliseconds(1000)) {}

inline void CpuTimer::start() {
    started = true;
    startTime = std::clock();
}

inline void CpuTimer::stop() {
    started = false;
    stopTime = std::clock();
}

inline double CpuTimer::elapsed_milliseconds() {
    double t = (double)(std::clock() - startTime) * 1000.0 / CLOCKS_PER_SEC;
    return t;
}

inline void CpuTimer::logElapsedTime(std::string tag) {
    LOGI(std::string("CpuTimer").append(tag).c_str(), "Elapsed time: %f ms", measurements / numMeasurements);
}

inline void CpuTimer::measure() {
    if (started) {
        auto elapsedWallTime = std::chrono::steady_clock::now() - startWallTime;
        stop();
        countMeasurement();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(elapsedWallTime) >= displayFrequency) {
            logElapsedTime();
            reset();
            startWallTime = std::chrono::steady_clock::now();
        }
        start();
    } else {
        start();
    }
}

inline void CpuTimer::countMeasurement() {
    numMeasurements++;
    measurements += elapsed_milliseconds();
}


inline void CpuTimer::reset() {
    numMeasurements = 0;
    measurements = 0.0;
}

#endif //LAGRANGIAN_FLUID_SIMULATION_CPU_TIMER_H
