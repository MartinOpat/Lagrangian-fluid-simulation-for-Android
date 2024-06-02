//
// Created by martin on 31-05-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_TIMER_H
#define LAGRANGIAN_FLUID_SIMULATION_TIMER_H

#include <chrono>

#include "consts.h"
#include "android_logging.h"

/**
 * @class Timer
 * @brief This class provides utilities for timing and logging in the application.
 *
 */
class Timer {
public:
    /**
     * @brief Constructor.
     */
    Timer();

    /**
     * @brief Starts the timer.
     */
    void start();

    /**
     * @brief Stops the timer.
     */
    void stop();

    /**
     * @brief Getter for the elapsed time in seconds.
     *
     * @return The elapsed time in seconds as a float.
     */
    float getElapsedTimeInSeconds();

    /**
     * @brief Logs the frames per second (FPS).
     */
    void logFPS();

    /**
     * @brief Logs the elapsed time.
     */
    void logElapsedTime();

    /**
     * @brief Checks if the timer is started.
     *
     * @return True if the timer is started, false otherwise.
     */
    bool isStarted() {return started;};

    /**
     * @brief Utility function that measures the time since its last calls and logs
     * the FPS and elapsed time (logging happens at most every `displayFrequency` seconds).
     */
    void measure();

private:
    bool started;  // Is timer started?

    std::chrono::time_point<std::chrono::steady_clock> startTime;
    std::chrono::time_point<std::chrono::steady_clock> stopTime;

    /**
     * @brief Gets the elapsed time.

     * @return The elapsed time as a chrono::seconds object.
     */
    std::chrono::seconds getElapsedTime();

    int numMeasurements;  // Number of measurements performed by `measure()` since last log.
    std::chrono::seconds displayFrequency;  // The max. allowed frequency of logging measurements.
};

#endif //LAGRANGIAN_FLUID_SIMULATION_TIMER_H
