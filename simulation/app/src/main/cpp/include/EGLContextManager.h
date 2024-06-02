//
// Created by martin on 01-06-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_EGLCONTEXTMANAGER_H
#define LAGRANGIAN_FLUID_SIMULATION_EGLCONTEXTMANAGER_H

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl32.h>
#include <thread>
#include <atomic>
#include <functional>

#include "android_logging.h"
#include "ThreadPool.h"

/**
 * @class EGLContextManager
 * @brief Class to manage the EGL context and shared context
 */
class EGLContextManager {
public:

    /**
     * @brief Constructor
     */
    EGLContextManager();

    /**
     * @brief Initializes EGL context and fill the member variables
     */
    void initContext();

    /**
     * @brief Synchronizes the EGL context between threads
     * @param threadPool A pointer to the ThreadPool to synchronize the context with
     */
    void syncEGLContext(ThreadPool *threadPool);

    std::atomic<GLsync> globalFence{nullptr};  // Atomic GLsync object for global fence synchronization.

private:
    EGLDisplay storedEglDisplay;
    EGLContext storedEglContext;

    EGLContext sharedContext;
    EGLConfig config;
    EGLint numConfigs;

    const EGLint contextAttributes[3];
    const EGLint configAttributes[15];
};

#endif //LAGRANGIAN_FLUID_SIMULATION_EGLCONTEXTMANAGER_H
