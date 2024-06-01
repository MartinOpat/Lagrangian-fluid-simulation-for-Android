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
#include <thread>
#include <atomic>
#include <functional>

#include "android_logging.h"
#include "netcdf_reader.h"
#include "mainview.h"
#include "particle.h"
#include "particles_handler.h"
#include "vector_field_handler.h"
#include "touch_handler.h"
#include "timer.h"
#include "ThreadPool.h"

EGLDisplay storedEglDisplay;
EGLContext storedEglContext;

const EGLint contextAttributes[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,  // Targeting OpenGL ES 3.0
        EGL_NONE
};
EGLContext sharedContext;
EGLConfig config;
EGLint numConfigs;

const EGLint configAttributes[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_NONE
};

std::vector<int> fileDescriptors;

Mainview* mainview;
ParticlesHandler* particlesHandler;
VectorFieldHandler* vectorFieldHandler;
TouchHandler* touchHandler;
Physics* physics;
Timer* timer;
ThreadPool *threadPool;

std::atomic<GLsync> globalFence{nullptr};


int currentFrame = 0;
int numFrames = 0;
float aspectRatio = 1.0f;

float global_time_in_step = 0.0f;

void loadStepHelper(int fdU, int fdV, int fdW) {
    NetCDFReader reader;
    std::string tempFileU = reader.writeTempFileFromFD(fdU, "tempU.nc");
    std::string tempFileV = reader.writeTempFileFromFD(fdV, "tempV.nc");
    std::string tempFileW = reader.writeTempFileFromFD(fdW, "tempW.nc");

    if (tempFileU.empty() || tempFileV.empty() || tempFileW.empty()) {
        LOGE("native-lib", "Failed to create temporary files.");
        return;
    }

    if (vectorFieldHandler == nullptr) {
        LOGE("native-lib", "Vector field handler not initialized");
    }
    vectorFieldHandler->loadTimeStep(tempFileU, tempFileV, tempFileW);
}

void loadStep(int frame) {
    LOGI("native-lib", "Loading step %d", frame);
    loadStepHelper(fileDescriptors[frame], fileDescriptors[numFrames + frame], fileDescriptors[2*numFrames + frame]);
}

void loadInitStep() {
    if (numFrames == 0) {
        LOGE("native-lib", "No frames loaded");
        return;
    } else if (numFrames == 1) {
        loadStep(0);
    } else if (numFrames == 2) {
        loadStep(0);
        loadStep(1);
        currentFrame = 1;
    } else {
        loadStep(0);
        loadStep(1);
        loadStep(2);
        currentFrame = 2;
    }
}

void update() {
    static auto lastUpdate = std::chrono::steady_clock::now(); // Last update time
    static const std::chrono::seconds updateInterval(TIME_STEP_IN_SECONDS);

    static auto lastCall = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    global_time_in_step += physics->dt;
    lastCall = now;

    if (global_time_in_step >= TIME_STEP_IN_SECONDS) {
        lastUpdate = now;
        global_time_in_step = 0.0f;

        auto start = std::chrono::steady_clock::now();
        GLsync fence = globalFence.load(std::memory_order_acquire);
        if (fence != nullptr) {
            LOGI("native-lib", "Waiting for fence");
            GLenum result = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000000);
            if (result == GL_TIMEOUT_EXPIRED || result == GL_WAIT_FAILED) {
                LOGI("native-lib", "Fence wait failed");
            }
            glDeleteSync(fence); // Clean up the fence object
            globalFence.store(nullptr, std::memory_order_release);
        } else {
            LOGI("native-lib", "No fence to wait for - oyoy");
        }
        auto end = std::chrono::steady_clock::now();
        LOGI("native-lib", "Time to wait for fence: %lld", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
        start = std::chrono::steady_clock::now();
        vectorFieldHandler->updateTimeStep();
        end = std::chrono::steady_clock::now();
        LOGI("native-lib", "Time to update vector field: %lld", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
        start = std::chrono::steady_clock::now();
        mainview->loadComputeBuffer(vectorFieldHandler->getOldVertices(), vectorFieldHandler->getNewVertices());
        end = std::chrono::steady_clock::now();
        LOGI("native-lib", "Time to load compute buffer: %lld", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
        start = std::chrono::steady_clock::now();

        currentFrame = (currentFrame + 1) % numFrames;
        threadPool->enqueue([frame = currentFrame]() {
            loadStep(frame);
            if (!eglMakeCurrent(storedEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, sharedContext)) {
                LOGE("native-lib", "Failed to make context current on thread");
                return;
            }
            mainview->preloadComputeBuffer(vectorFieldHandler->getFutureVertices(), globalFence);
            eglMakeCurrent(storedEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        });
        end = std::chrono::steady_clock::now();
        LOGI("native-lib", "Time to load step: %lld", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    }

    timer->measure();
}


void init() {
    touchHandler = new TouchHandler(mainview->getTransforms());
    vectorFieldHandler = new VectorFieldHandler();
    physics = new Physics(*vectorFieldHandler, Physics::Model::particles_advection);

    particlesHandler = new ParticlesHandler(ParticlesHandler::InitType::line ,*physics, NUM_PARTICLES);  // Code-wise initialization
//    particlesHandler = new ParticlesHandler(*physics, NUM_PARTICLES);  // Initialization from file

    timer = new Timer();
    threadPool = new ThreadPool(1);

    LOGI("native-lib", "init complete");
}

extern "C" {
    JNIEXPORT void JNICALL Java_com_rug_lagrangianfluidsimulation_MainActivity_drawFrame(JNIEnv* env, jobject /* this */) {
        update();
        mainview->setFrame();
        vectorFieldHandler->draw(*mainview);

        mainview->dispatchComputeShader(physics->dt, global_time_in_step, vectorFieldHandler->getWidth(), vectorFieldHandler->getHeight(), vectorFieldHandler->getDepth());
        particlesHandler->drawParticles(*mainview);
    }

    JNIEXPORT void JNICALL Java_com_rug_lagrangianfluidsimulation_MainActivity_setupGraphics(JNIEnv* env, jobject obj, jobject assetManager) {
        mainview = new Mainview(AAssetManager_fromJava(env, assetManager));
        mainview->setupGraphics();
        mainview->getTransforms().setAspectRatio(aspectRatio);

        init();
        LOGI("native-lib", "Graphics setup complete");

        storedEglDisplay = eglGetCurrentDisplay();
        storedEglContext = eglGetCurrentContext();

        if (!eglChooseConfig(storedEglDisplay, configAttributes, &config, 1, &numConfigs)) {
            LOGE("native-lib", "Failed to choose config");
            return;
        }

        sharedContext = eglCreateContext(storedEglDisplay, config, storedEglContext, contextAttributes);
    }

    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_FileAccessHelper_loadNetCDFData(
            JNIEnv* env, jobject /* this */, jint fdU, jint fdV) {

        NetCDFReader reader;
        std::string tempFileU = reader.writeTempFileFromFD(fdU, "tempU.nc");
        std::string tempFileV = reader.writeTempFileFromFD(fdV, "tempV.nc");

        if (tempFileU.empty() || tempFileV.empty()) {
            LOGE("native-lib", "Failed to create temporary files.");
            return;
        }

        vectorFieldHandler->loadTimeStep(tempFileU, tempFileV);

        LOGI("native-lib", "Particles initialized");
    }

    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_FileAccessHelper_loadFilesFDs(
                JNIEnv* env, jobject /* this */, jintArray jfds) {
        LOGI("native-lib", "Loading file descriptors");
        jsize len = env->GetArrayLength(jfds);
        numFrames = len / 3;
        LOGI("native-lib", "Number of frames: %d", numFrames);

        jint* fds = env->GetIntArrayElements(jfds, nullptr);
        fileDescriptors.clear();
        for (int i = 0; i < len; i++) {
            fileDescriptors.push_back(fds[i]);
        }

        env->ReleaseIntArrayElements(jfds, fds, 0);
        LOGI("native-lib", "File descriptors loaded");
        loadInitStep();
        LOGI("native-lib", "Initial step loaded");
    }


    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_FileAccessHelper_loadNetCDFData3D(
            JNIEnv* env, jobject /* this */, jint fdU, jint fdV, jint fdW) {

        NetCDFReader reader;
        std::string tempFileU = reader.writeTempFileFromFD(fdU, "tempU.nc");
        std::string tempFileV = reader.writeTempFileFromFD(fdV, "tempV.nc");
        std::string tempFileW = reader.writeTempFileFromFD(fdW, "tempW.nc");

        if (tempFileU.empty() || tempFileV.empty() || tempFileW.empty()) {
            LOGE("native-lib", "Failed to create temporary files.");
            return;
        }


        vectorFieldHandler = new VectorFieldHandler();
        vectorFieldHandler->loadTimeStep(tempFileU, tempFileV, tempFileW);
        LOGI("native-lib", "NetCDF files loaded");

        physics = new Physics(*vectorFieldHandler, Physics::Model::particles_advection);

        particlesHandler = new ParticlesHandler(ParticlesHandler::InitType::line, *physics);
        LOGI("native-lib", "Particles initialized");
    }

    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_MainActivity_createBuffers(JNIEnv *env, jobject thiz) {
        mainview->createVectorFieldBuffer(vectorFieldHandler->getOldVertices());
        mainview->createParticlesBuffer(particlesHandler->getParticlesPositions());
        mainview->createComputeBuffer(vectorFieldHandler->getOldVertices(), vectorFieldHandler->getNewVertices(), vectorFieldHandler->getFutureVertices());
        LOGI("native-lib", "Buffers created");
    }

    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_MainActivity_nativeSendTouchEvent(JNIEnv *env, jobject obj, jint pointerCount, jfloatArray xArray, jfloatArray yArray, jint action) {
        jfloat* xTemp = env->GetFloatArrayElements(xArray, nullptr);
        jfloat* yTemp = env->GetFloatArrayElements(yArray, nullptr);

        if (pointerCount == 1) {
            touchHandler->handleSingleTouch(xTemp[0], yTemp[0], action);
        } else if (pointerCount == 2) {
            float x[2] = {xTemp[0], xTemp[1]};
            float y[2] = {yTemp[0], yTemp[1]};
            touchHandler->handleDoubleTouch(x, y, action);
        }

        LOGI("native-lib", "Touch event: %d", action);

        env->ReleaseFloatArrayElements(xArray, xTemp, 0);
        env->ReleaseFloatArrayElements(yArray, yTemp, 0);
    }

    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_MainActivity_onDestroyNative(JNIEnv *env, jobject thiz) {
        delete mainview;
        delete particlesHandler;
        delete vectorFieldHandler;
        delete physics;
        delete touchHandler;
        delete timer;
        delete threadPool;
    }

    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_MainActivity_loadDeviceInfo(JNIEnv *env, jobject thiz, jdouble jaspectRatio) {
        aspectRatio = (float) jaspectRatio;
    }

    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_FileAccessHelper_loadInitialPositions(JNIEnv *env, jobject thiz, jint fd) {
        if (particlesHandler->areParticlesInitialized()) {
            LOGI("native-lib", "Particles already initialized");
            return;
        }
        LOGI("native-lib", "Loading initial positions");
        NetCDFReader reader;
        std::string tempFile = reader.writeTempFileFromFD(fd, "temp.nc");

        if (tempFile.empty()) {
            LOGE("native-lib", "Failed to create temporary file.");
            return;
        }

        particlesHandler->loadPositionsFromFile(tempFile);
        mainview->loadParticlesData(particlesHandler->getParticlesPositions());
        LOGI("native-lib", "Particles initialized");
    }
} // extern "C"
