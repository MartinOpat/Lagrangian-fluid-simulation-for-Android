//
// Created by martin on 12-06-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_GPU_TIMER_H
#define LAGRANGIAN_FLUID_SIMULATION_GPU_TIMER_H

//#define GL_GLEXT_PROTOTYPES
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
    void logElapsedTime(std::string tag="");
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

// Define the function pointer type
typedef void (GL_APIENTRYP PFNGLGETQUERYOBJECTUI64VEXTPROC)(GLuint id, GLenum pname, GLuint64* params);

// Declare the function pointer
PFNGLGETQUERYOBJECTUI64VEXTPROC glGetQueryObjectui64vEXT;

inline GpuTimer::GpuTimer()
        : started(false), currentQuery(0), numMeasurements(0), displayFrequency(1000.0) {
    glGenQueries(2, queryID);
    glGetQueryObjectui64vEXT = (PFNGLGETQUERYOBJECTUI64VEXTPROC)eglGetProcAddress("glGetQueryObjectui64vEXT");
}

inline GpuTimer::~GpuTimer() {
    glDeleteQueries(2, queryID);
}

inline void GpuTimer::start() {
    if (!started) {
        started = true;
        glBeginQuery(GL_TIME_ELAPSED_EXT, queryID[currentQuery]);
    }
}

inline void GpuTimer::stop() {
    if (started) {
        started = false;
        glEndQuery(GL_TIME_ELAPSED_EXT);
        glFinish();  // Ensure all rendering commands are completed
        int nextQuery = (currentQuery + 1) % 2;
        GLuint64 elapsed_time;

//        glGetQueryObjectuiv(queryID[currentQuery], GL_QUERY_RESULT_EXT, &elapsed_time);
        GLuint available = 0;
        while (!available) {
            glGetQueryObjectuiv(queryID[currentQuery], GL_QUERY_RESULT_AVAILABLE, &available);
        }
        glGetQueryObjectui64vEXT(queryID[currentQuery], GL_QUERY_RESULT, &elapsed_time);

        times.push_back(elapsed_time / 1000000.0); // Convert nanoseconds to milliseconds
        currentQuery = nextQuery;
    }
}

inline double GpuTimer::elapsedMilliseconds() {
    double t = times.empty() ? 0.0 : times.back();
    return t;
}

inline void GpuTimer::logElapsedTime(std::string tag) {
    LOGI(std::string("GpuTimer").append(tag).c_str(), "Elapsed time: %f ms", measurements / numMeasurements);
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
