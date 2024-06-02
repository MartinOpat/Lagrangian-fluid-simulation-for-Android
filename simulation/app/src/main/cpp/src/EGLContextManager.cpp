//
// Created by martin on 01-06-2024.
//

#include "include/EGLContextManager.h"

// Constructor
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
    // Retrieve the current EGL display and context
    storedEglDisplay = eglGetCurrentDisplay();
    storedEglContext = eglGetCurrentContext();

    // Choose a config
    if (!eglChooseConfig(storedEglDisplay, configAttributes, &config, 1, &numConfigs)) {
        LOGE("EGLContextManager", "Failed to choose config");
        return;
    }

    // Generate a shared context
    sharedContext = eglCreateContext(storedEglDisplay, config, storedEglContext, contextAttributes);
}

void EGLContextManager::syncEGLContext(ThreadPool *threadPool) {
    GLsync fence = globalFence.load(std::memory_order_acquire);
    if (fence != nullptr) {
        // Fence active, wait for it to complete
        GLenum result = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000000);
        if (result == GL_TIMEOUT_EXPIRED || result == GL_WAIT_FAILED) {
            LOGI("EGLContextManager", "Fence wait failed");
        }
        glDeleteSync(fence); // Clean up the fence object
        globalFence.store(nullptr, std::memory_order_release);
    } else {
        // Fence not active, make context current on thread
        threadPool->enqueue([this]() {
            if (!eglMakeCurrent(storedEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, sharedContext)) {
                LOGE("EGLContextManager", "Failed to make context current on thread");
                return;
            }
        });
    }
}
