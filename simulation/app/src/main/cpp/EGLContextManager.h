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

class EGLContextManager {
public:
    EGLContextManager();

    void initContext();
    void syncEGLContext(ThreadPool *threadPool);

    EGLDisplay& getDisplay() { return storedEglDisplay; }
    EGLContext& getContext() { return storedEglContext; }
    EGLContext& getSharedContext() { return sharedContext; }

    std::atomic<GLsync> globalFence{nullptr};

private:
    EGLDisplay storedEglDisplay;
    EGLContext storedEglContext;

    const EGLint contextAttributes[3];

    EGLContext sharedContext;
    EGLConfig config;
    EGLint numConfigs;

    const EGLint configAttributes[15];
};

#endif //LAGRANGIAN_FLUID_SIMULATION_EGLCONTEXTMANAGER_H
