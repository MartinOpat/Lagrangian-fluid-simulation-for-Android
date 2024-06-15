//
// Created by martin on 01-06-2024.
//

#include "include/EGLContextManager.h"

EGLContextManager::EGLContextManager() :
        contextAttributes{EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE},
        configAttributes{EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
                         EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                         EGL_RED_SIZE, 8,
                         EGL_GREEN_SIZE, 8,
                         EGL_BLUE_SIZE, 8,
                         EGL_ALPHA_SIZE, 8,
                         EGL_DEPTH_SIZE, 24,
                         EGL_NONE} {}

EGLContextManager::~EGLContextManager() {
    destroyContext();
}

void EGLContextManager::initContext(EGLNativeWindowType window) {
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, NULL, NULL);

    if (!eglChooseConfig(display, configAttributes, &config, 1, &numConfigs) || numConfigs < 1) {
        LOGE("EGLContextManager", "Failed to choose config");
        return;
    }

    context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttributes);
    if (context == EGL_NO_CONTEXT) {
        LOGE("EGLContextManager", "Failed to create EGL context");
        return;
    }

    surface = eglCreateWindowSurface(display, config, window, NULL);
    if (surface == EGL_NO_SURFACE) {
        LOGE("EGLContextManager", "Failed to create EGL surface");
        return;
    }

    if (!eglMakeCurrent(display, surface, surface, context)) {
        LOGE("EGLContextManager", "Failed to make context current");
        return;
    }

    LOGI("EGLContextManager", "EGL context initialized");
}

void EGLContextManager::syncEGLContext(ThreadPool *threadPool) {
    GLsync fence = globalFence.load(std::memory_order_acquire);
    if (fence != nullptr) {
        GLenum result = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000000);
        if (result == GL_TIMEOUT_EXPIRED || result == GL_WAIT_FAILED) {
            LOGI("EGLContextManager", "Fence wait failed");
        }
        glDeleteSync(fence);
        globalFence.store(nullptr, std::memory_order_release);
    } else {
        threadPool->enqueue([this]() {
            if (!eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context)) {
                LOGE("EGLContextManager", "Failed to make context current on thread");
            }
        });
    }
}

void EGLContextManager::swapBuffers() {
    if (!eglSwapBuffers(display, surface)) {
        LOGE("EGLContextManager", "Failed to swap buffers");
    }
}

void EGLContextManager::destroyContext() {
    if (display != EGL_NO_DISPLAY) {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (context != EGL_NO_CONTEXT) {
            eglDestroyContext(display, context);
        }
        if (surface != EGL_NO_SURFACE) {
            eglDestroySurface(display, surface);
        }
        eglTerminate(display);
    }
    display = EGL_NO_DISPLAY;
    context = EGL_NO_CONTEXT;
    surface = EGL_NO_SURFACE;
}
