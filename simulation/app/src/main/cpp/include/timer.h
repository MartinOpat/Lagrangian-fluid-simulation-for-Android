//
// Created by martin on 31-05-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_TIMER_H
#define LAGRANGIAN_FLUID_SIMULATION_TIMER_H

#include <chrono>
#include <ctime>

#include "consts.h"
#include "android_logging.h"

/**
 * @class Timer
 * @brief Templated class providing utilities for timing and logging in the application.
 *
 * @tparam ClockType The type of clock to use for timing.
 */
template<typename ClockType>
class Timer {
public:
    /**
     * @brief Constructor for the Timer class.
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
     * @brief Gets the elapsed time in seconds.
     *
     * @return The elapsed time in seconds.
     */
    float getElapsedTimeInSeconds();

    /**
     * @brief Gets the elapsed time in milliseconds.
     *
     * @return The elapsed time in milliseconds.
     */
    std::chrono::milliseconds getElapsedTime();

    /**
     * @brief Logs the frames per second to console.
     */
    void logFPS();

    /**
     * @brief Logs the elapsed time to console.
     */
    void logElapsedTime();

    /**
     * @brief Checks if the timer is started.
     *
     * @return True if the timer is started, false otherwise.
     */
    bool isStarted();

    /**
     * @brief Measures the time elapsed since the last measurement.
     * @note This method logs the elapsed time to console every `displayFrequency` milliseconds.
     */
    void measure();

private:
    bool started;
    std::chrono::time_point<ClockType> startTime, stopTime;
    int numMeasurements;
    std::chrono::milliseconds displayFrequency;
};

// Definitions are below the class declaration, but still in the header

template<typename ClockType>
inline Timer<ClockType>::Timer()
        : started(false), numMeasurements(0), displayFrequency(std::chrono::milliseconds(1000)) {}

template<typename ClockType>
inline void Timer<ClockType>::start() {
    started = true;
    numMeasurements = 0;
    startTime = ClockType::now();
}

template<typename ClockType>
inline void Timer<ClockType>::stop() {
    started = false;
    stopTime = ClockType::now();
}

template<typename ClockType>
inline float Timer<ClockType>::getElapsedTimeInSeconds() {
    auto elapsedTime = ClockType::now() - startTime;
    std::chrono::duration<float> seconds = elapsedTime;
    return seconds.count();
}

template<typename ClockType>
inline std::chrono::milliseconds Timer<ClockType>::getElapsedTime() {
    auto elapsedTime = ClockType::now() - startTime;
    return std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime);
}

template<typename ClockType>
inline void Timer<ClockType>::logFPS() {
    float elapsedTime = getElapsedTimeInSeconds();
    float fps = numMeasurements / elapsedTime;
    LOGI("Timer", "FPS: %f", fps);
}

template<typename ClockType>
inline void Timer<ClockType>::logElapsedTime() {
    float elapsedTime = getElapsedTimeInSeconds() * 1000 / numMeasurements;  // Convert to milliseconds
    LOGI("Timer", "Elapsed time: %f ms", elapsedTime);
}

template<typename ClockType>
inline bool Timer<ClockType>::isStarted() {
    return started;
}

template<typename ClockType>
inline void Timer<ClockType>::measure() {
    if (isStarted()) {
        numMeasurements++;
        if (getElapsedTime() >= displayFrequency) {
            stop();
//            logFPS();
            logElapsedTime();
            start();
        }
    } else {
        start();
    }
}

#endif //LAGRANGIAN_FLUID_SIMULATION_TIMER_H
