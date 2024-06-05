//
// Created by martin on 31-05-2024.
//

#include "include/timer.h"

Timer::Timer() {
    started = false;
    numMeasurements = 0;
    displayFrequency = std::chrono::seconds(1);
}

void Timer::start() {
    started = true;
    numMeasurements = 0;
    startTimeSteady = std::chrono::steady_clock::now();
    startTimeHighRes = std::chrono::high_resolution_clock::now();
    startTimeCpu = std::clock();
}

void Timer::stop() {
    started = false;
    stopTimeSteady = std::chrono::steady_clock::now();
    stopTimeHighRes = std::chrono::high_resolution_clock::now();
    stopTimeCpu = std::clock();
}

std::chrono::seconds Timer::getSteadyElapsedTime() {
    stopTimeSteady = std::chrono::steady_clock::now();
    auto elapsedTime = stopTimeSteady - startTimeSteady;
    return std::chrono::duration_cast<std::chrono::seconds>(elapsedTime);
}

std::chrono::seconds Timer::getHighResElapsedTime() {
    stopTimeHighRes = std::chrono::high_resolution_clock::now();
    auto elapsedTime = stopTimeHighRes - startTimeHighRes;
    return std::chrono::duration_cast<std::chrono::seconds>(elapsedTime);
}

std::clock_t Timer::getCpuElapsedTime() {
    stopTimeCpu = std::clock();
    return stopTimeCpu - startTimeCpu;
}

float Timer::getSteadyElapsedTimeInSeconds() {
    if (started) {
        stopTimeSteady = std::chrono::steady_clock::now();
    }
    std::chrono::duration<float> elapsedTime = stopTimeSteady - startTimeSteady;
    return elapsedTime.count();
}

float Timer::getHighResElapsedTimeInSeconds() {
    if (started) {
        stopTimeHighRes = std::chrono::high_resolution_clock::now();
    }
    std::chrono::duration<float> elapsedTime = stopTimeHighRes - startTimeHighRes;
    return elapsedTime.count();
}

float Timer::getCpuElapsedTimeInSeconds() {
    if (started) {
        stopTimeCpu = std::clock();
    }
    return (stopTimeCpu - startTimeCpu) / (float) CLOCKS_PER_SEC;
}

void Timer::logFPS() {
    float elapsedTime = getSteadyElapsedTimeInSeconds();
    float elapsedTimePerMeasurement = elapsedTime / numMeasurements;
    LOGI("timer", "steady FPS: %f", 1.0f / elapsedTimePerMeasurement);

    elapsedTime = getHighResElapsedTimeInSeconds();
    elapsedTimePerMeasurement = elapsedTime / numMeasurements;
    LOGI("timer", "high res FPS: %f", 1.0f / elapsedTimePerMeasurement);

    elapsedTime = getCpuElapsedTimeInSeconds();
    elapsedTimePerMeasurement = elapsedTime / numMeasurements;
    LOGI("timer", "CPU FPS: %f", 1.0f / elapsedTimePerMeasurement);
}

void Timer::logElapsedTime() {
    float elapsedTime = getSteadyElapsedTimeInSeconds();
    float elapsedTimePerMeasurement = elapsedTime / numMeasurements;
    LOGI("timer", "steady Elapsed time: %f ms", 1000*elapsedTimePerMeasurement);

    elapsedTime = getHighResElapsedTimeInSeconds();
    elapsedTimePerMeasurement = elapsedTime / numMeasurements;
    LOGI("timer", "high res Elapsed time: %f ms", 1000*elapsedTimePerMeasurement);

    elapsedTime = getCpuElapsedTimeInSeconds();
    elapsedTimePerMeasurement = elapsedTime / numMeasurements;
    LOGI("timer", "CPU Elapsed time: %f ms", 1000*elapsedTimePerMeasurement);
}

void Timer::measure() {
    if (isStarted()) {
        numMeasurements++;
        if (getSteadyElapsedTime() > displayFrequency) {
            stop();
            logFPS();
            logElapsedTime();
            start();
        }
    } else {
        start();
    }
}