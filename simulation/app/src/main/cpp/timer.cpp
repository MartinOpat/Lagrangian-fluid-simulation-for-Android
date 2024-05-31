//
// Created by martin on 31-05-2024.
//

#include "timer.h"

Timer::Timer() {
    started = false;
    numMeasurements = 0;
    displayFrequency = std::chrono::seconds(1);
}

void Timer::start() {
    started = true;
    numMeasurements = 0;
    startTime = std::chrono::steady_clock::now();
}

void Timer::stop() {
    started = false;
    stopTime = std::chrono::steady_clock::now();
}

std::chrono::seconds Timer::getElapsedTime() {
    stopTime = std::chrono::steady_clock::now();
    auto elapsedTime = stopTime - startTime;
    return std::chrono::duration_cast<std::chrono::seconds>(elapsedTime);
}

float Timer::getElapsedTimeInSeconds() {
    if (started) {
        stopTime = std::chrono::steady_clock::now();
    }
    std::chrono::duration<float> elapsedTime = stopTime - startTime;
    return elapsedTime.count();
}

void Timer::logFPS() {
    float elapsedTime = getElapsedTimeInSeconds();
    float elapsedTimePerMeasurement = elapsedTime / numMeasurements;
    LOGI("timer", "FPS: %f", 1.0f / elapsedTimePerMeasurement);
}

void Timer::logElapsedTime() {
    float elapsedTime = getElapsedTimeInSeconds();
    float elapsedTimePerMeasurement = elapsedTime / numMeasurements;
    LOGI("timer", "Elapsed time: %f ms", 1000*elapsedTimePerMeasurement);
}

void Timer::measure() {
    if (isStarted()) {
        if (getElapsedTime() > displayFrequency) {
            stop();
            logFPS();
            logElapsedTime();
            start();
        } else {
            numMeasurements++;
        }
    } else {
        start();
    }
}