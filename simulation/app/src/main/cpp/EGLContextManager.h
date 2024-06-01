//
// Created by martin on 01-06-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_EGLCONTEXTMANAGER_H
#define LAGRANGIAN_FLUID_SIMULATION_EGLCONTEXTMANAGER_H

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "android_logging.h"

class EGLContextManager {
public:
    EGLContextManager();

    void initContext();

    EGLDisplay& getDisplay() { return storedEglDisplay; }
    EGLContext& getContext() { return storedEglContext; }
    EGLContext& getSharedContext() { return sharedContext; }

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
