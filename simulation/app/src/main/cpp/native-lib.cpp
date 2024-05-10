#include <jni.h>
#include <string>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>
#include <chrono>
#include <netcdf>
#include <assert.h>
#include <GLES3/gl32.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "android_logging.h"
#include "netcdf_reader.h"
#include "mainview.h"
#include "particle.h"
#include "particles_handler.h"
#include "vector_field_handler.h"
#include "touch_handler.h"


bool started = false;

GLShaderManager* shaderManager;
ParticlesHandler* particlesHandler;
VectorFieldHandler* vectorFieldHandler;
TouchHandler* touchHandler;


int frameCount = 0;
float timeCount = 0.0f;

void updateFrame() {
    static auto lastUpdate = std::chrono::steady_clock::now(); // Last update time
    static const std::chrono::seconds updateInterval(1);       // Update every 1 second

//    auto now = std::chrono::steady_clock::now();
//    if (now - lastUpdate >= updateInterval) {                  // Check if 1 second has passed
//        currentFrame = (currentFrame + 1) % allVertices.size(); // Update the frame index
//        lastUpdate = now;                                      // Reset the last update time
//    }
}


bool setupOpenGLES32(EGLDisplay eglDisplay, EGLNativeWindowType nativeWindow) {
    const EGLint configAttributes[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE
    };

    EGLConfig eglConfig;
    EGLint numConfigs;
    if (!eglChooseConfig(eglDisplay, configAttributes, &eglConfig, 1, &numConfigs)) {
        return false;
    }

    EGLSurface eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, nativeWindow, NULL);
    if (eglSurface == EGL_NO_SURFACE) {
        return false;
    }

    const EGLint contextAttributes[] = {
            EGL_CONTEXT_CLIENT_VERSION, 3,
            EGL_NONE
    };

    EGLContext eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, contextAttributes);
    if (eglContext == EGL_NO_CONTEXT) {
        return false;
    }

    if (!eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
        return false;
    }

    return true;
}

extern "C" {
    JNIEXPORT void JNICALL Java_com_example_lagrangianfluidsimulation_MainActivity_drawFrame(JNIEnv* env, jobject /* this */) {
        shaderManager->setFrame();
        vectorFieldHandler->draw(*shaderManager);

//        particlesHandler->drawParticles(*shaderManager);

        //        updateFrame();
        frameCount++;
    }

    JNIEXPORT void JNICALL Java_com_example_lagrangianfluidsimulation_MainActivity_setupGraphics(JNIEnv* env, jobject obj, jobject assetManager, jobject surface) {
        EGLDisplay eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (eglDisplay == EGL_NO_DISPLAY) {
            LOGE("eglGetDisplay() returned error %d", eglGetError());
        }

        if (!eglInitialize(eglDisplay, 0, 0)) {
            LOGE("eglInitialize() returned error %d", eglGetError());
        }

        EGLNativeWindowType nativeWindow = ANativeWindow_fromSurface(env, surface);
        if (!setupOpenGLES32(eglDisplay, nativeWindow)) {
            LOGE("Failed to setup OpenGL ES 3.2");
        }


        shaderManager = new GLShaderManager(AAssetManager_fromJava(env, assetManager));
        shaderManager->setupGraphics();
        touchHandler = new TouchHandler(*shaderManager);
        LOGI("Graphics setup complete");
    }

    JNIEXPORT void JNICALL
    Java_com_example_lagrangianfluidsimulation_FileAccessHelper_initializeNetCDFVisualization(
            JNIEnv* env, jobject /* this */, jint fdU, jint fdV) {

        NetCDFReader reader;
        std::string tempFileU = reader.writeTempFileFromFD(fdU, "tempU.nc");
        std::string tempFileV = reader.writeTempFileFromFD(fdV, "tempV.nc");

        if (tempFileU.empty() || tempFileV.empty()) {
            LOGE("Failed to create temporary files.");
            return;
        }

        vectorFieldHandler = new VectorFieldHandler();
        vectorFieldHandler->loadAllTimeSteps(tempFileU, tempFileV);

        particlesHandler = new ParticlesHandler(ParticlesHandler::InitType::two_lines, *vectorFieldHandler);
        LOGI("Particles initialized");
    }

    JNIEXPORT void JNICALL
    Java_com_example_lagrangianfluidsimulation_FileAccessHelper_initializeNetCDFVisualization3D(
            JNIEnv* env, jobject /* this */, jint fdU, jint fdV, jint fdW) {

        NetCDFReader reader;
        std::string tempFileU = reader.writeTempFileFromFD(fdU, "tempU.nc");
        std::string tempFileV = reader.writeTempFileFromFD(fdV, "tempV.nc");
        std::string tempFileW = reader.writeTempFileFromFD(fdW, "tempW.nc");

        if (tempFileU.empty() || tempFileV.empty() || tempFileW.empty()) {
            LOGE("Failed to create temporary files.");
            return;
        }

        vectorFieldHandler = new VectorFieldHandler();
        vectorFieldHandler->loadAllTimeSteps(tempFileU, tempFileV, tempFileW);
        LOGI("NetCDF files loaded");

        particlesHandler = new ParticlesHandler(ParticlesHandler::InitType::line, *vectorFieldHandler);
        LOGI("Particles initialized");
    }

    JNIEXPORT void JNICALL
    Java_com_example_lagrangianfluidsimulation_MainActivity_createBuffers(JNIEnv *env, jobject thiz) {
        shaderManager->createVectorFieldBuffer(vectorFieldHandler->getAllVertices());
        shaderManager->createParticlesBuffer(particlesHandler->getParticlesPositions());
        LOGI("Buffers created");
    }

    JNIEXPORT void JNICALL
    Java_com_example_lagrangianfluidsimulation_MainActivity_nativeSendTouchEvent(JNIEnv *env, jobject obj, jint pointerCount, jfloatArray xArray, jfloatArray yArray, jint action) {
        jfloat* xTemp = env->GetFloatArrayElements(xArray, nullptr);
        jfloat* yTemp = env->GetFloatArrayElements(yArray, nullptr);

        if (pointerCount == 1) {
            touchHandler->handleSingleTouch(xTemp[0], yTemp[0], action);
        } else if (pointerCount == 2) {
            float x[2] = {xTemp[0], xTemp[1]};
            float y[2] = {yTemp[0], yTemp[1]};
            touchHandler->handleDoubleTouch(x, y, action);
        }

        LOGI("Touch event: %d", action);

        env->ReleaseFloatArrayElements(xArray, xTemp, 0);
        env->ReleaseFloatArrayElements(yArray, yTemp, 0);
    }
} // extern "C"
