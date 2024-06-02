//
// Created by martin on 31-05-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_TIMER_H
#define LAGRANGIAN_FLUID_SIMULATION_TIMER_H

#include <chrono>

#include "consts.h"
#include "android_logging.h"

class Timer {
public:
    Timer();

    void start();
    void stop();

    float getElapsedTimeInSeconds();

    void logFPS();
    void logElapsedTime();

    bool isStarted() {return started;};

    void measure();

private:
    bool started;
    std::chrono::time_point<std::chrono::steady_clock> startTime, stopTime;

    std::chrono::seconds getElapsedTime();
    int numMeasurements;
    std::chrono::seconds displayFrequency;

};

#endif //LAGRANGIAN_FLUID_SIMULATION_TIMER_H
