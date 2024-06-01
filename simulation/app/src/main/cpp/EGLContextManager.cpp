//
// Created by martin on 01-06-2024.
//

#include "EGLContextManager.h"

EGLContextManager::EGLContextManager():
    contextAttributes{EGL_CONTEXT_CLIENT_VERSION, 3,
                      EGL_NONE},
    configAttributes{EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
                     EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                     EGL_RED_SIZE, 8,
                     EGL_GREEN_SIZE, 8,
                     EGL_BLUE_SIZE, 8,
                     EGL_ALPHA_SIZE, 8,
                     EGL_DEPTH_SIZE, 24,
                     EGL_NONE} {

}

void EGLContextManager::initContext() {
    storedEglDisplay = eglGetCurrentDisplay();
    storedEglContext = eglGetCurrentContext();

    if (!eglChooseConfig(storedEglDisplay, configAttributes, &config, 1, &numConfigs)) {
        LOGE("native-lib", "Failed to choose config");
        return;
    }

    sharedContext = eglCreateContext(storedEglDisplay, config, storedEglContext, contextAttributes);
}
