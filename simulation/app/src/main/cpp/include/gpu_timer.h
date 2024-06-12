//
// Created by martin on 12-06-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_GPU_TIMER_H
#define LAGRANGIAN_FLUID_SIMULATION_GPU_TIMER_H

#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include "android_logging.h"
#include <vector>
#include <chrono>

class GpuTimer {
public:
    GpuTimer();
    ~GpuTimer();
    void start();
    void stop();
    double elapsedMilliseconds();
    void logElapsedTime();
    void countMeasurement();
    void reset();

private:
    bool started;
    GLuint queryID[2]; // We use two queries to manage overlapping intervals
    int currentQuery;
    int numMeasurements;
    double displayFrequency;
    std::vector<double> times;
    double measurements;
};

inline GpuTimer::GpuTimer()
        : started(false), currentQuery(0), numMeasurements(0), displayFrequency(1000.0) {
    glGenQueries(2, queryID);
}

inline GpuTimer::~GpuTimer() {
    glDeleteQueries(2, queryID);
}

inline void GpuTimer::start() {
    if (!started) {
        started = true;
        numMeasurements = 0;
        glBeginQuery(GL_TIME_ELAPSED_EXT, queryID[currentQuery]);
    }
}

inline void GpuTimer::stop() {
    if (started) {
        started = false;
        glEndQuery(GL_TIME_ELAPSED_EXT);
        int nextQuery = (currentQuery + 1) % 2;
        GLuint elapsed_time;
        glGetQueryObjectuiv(queryID[currentQuery], GL_QUERY_RESULT_EXT, &elapsed_time);
        times.push_back(elapsed_time / 1000000.0); // Convert nanoseconds to milliseconds
        currentQuery = nextQuery;
    }
}

inline double GpuTimer::elapsedMilliseconds() {
    double t = times.empty() ? 0.0 : times.back();
    return t;
}

inline void GpuTimer::logElapsedTime() {
    LOGI("GpuTimer", "Elapsed time: %f ms", elapsedMilliseconds() / numMeasurements);
}

inline void GpuTimer::countMeasurement() {
    numMeasurements++;
    measurements += elapsedMilliseconds();
}

inline void GpuTimer::reset() {
    times.clear();
    numMeasurements = 0;
    measurements = 0.0;
}



#endif //LAGRANGIAN_FLUID_SIMULATION_GPU_TIMER_H
