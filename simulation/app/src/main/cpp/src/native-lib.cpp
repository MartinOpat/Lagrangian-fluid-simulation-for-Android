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

#include "include/android_logging.h"
#include "include/netcdf_reader.h"
#include "include/mainview.h"
#include "include/particle.h"
#include "include/particles_handler.h"
#include "include/vector_field_handler.h"
#include "include/touch_handler.h"
#include "include/timer.h"
#include "include/ThreadPool.h"
#include "include/EGLContextManager.h"

std::vector<int> fileDescriptors;

Mainview* mainview;
ParticlesHandler* particlesHandler;
VectorFieldHandler* vectorFieldHandler;
TouchHandler* touchHandler;
Physics* physics;
Timer<std::chrono::steady_clock>* timer;
ThreadPool *threadPool;
EGLContextManager *eglContextManager;

// From consts.h
float global_time_in_step = 0.0f;
Mode mode;

// Rendering vars.
int currentFrame = 0;
int numFrames = 0;
float aspectRatio = 1.0f;


inline void loadStep(int frame) {
    vectorFieldHandler->loadTimeStep(fileDescriptors[frame], fileDescriptors[numFrames + frame], fileDescriptors[2*numFrames + frame]);
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


void check_update() {
    global_time_in_step += physics->dt;

    if (global_time_in_step >= TIME_STEP) {
        global_time_in_step = 0.0f;

        eglContextManager->syncEGLContext(threadPool);

        vectorFieldHandler->updateTimeStep();
        mainview->loadComputeBuffer();
        currentFrame = (currentFrame + 1) % numFrames;

        LOGI("native-lib", "Loading step %d", currentFrame);
        threadPool->enqueue([frame = currentFrame]() {
            loadStep(frame);
            mainview->preloadComputeBuffer(vectorFieldHandler->getFutureVertices(), eglContextManager->globalFence);
        });
    }

    timer->measure();
}


void init() {
    mode = Mode::computeShaders;

    touchHandler = new TouchHandler(mainview->getTransforms());
    vectorFieldHandler = new VectorFieldHandler(1, 1);
    physics = new Physics(*vectorFieldHandler, Physics::Model::particles_advection);

    particlesHandler = new ParticlesHandler(ParticlesHandler::InitType::uniform ,*physics, NUM_PARTICLES);  // Code-wise initialization
//    particlesHandler = new ParticlesHandler(*physics, NUM_PARTICLES);  // Initialization from file

    timer = new Timer<std::chrono::steady_clock>();

    threadPool = new ThreadPool(1);
    eglContextManager = new EGLContextManager();

    LOGI("native-lib", "init complete");
}

extern "C" {
    JNIEXPORT void JNICALL Java_com_rug_lagrangianfluidsimulation_MainActivity_drawFrame(JNIEnv* env, jobject /* this */) {
        check_update();
        particlesHandler->simulateParticles(*mainview);
        mainview->setFrame();

        vectorFieldHandler->draw(*mainview);
        particlesHandler->draw(*mainview);
    }

    JNIEXPORT void JNICALL Java_com_rug_lagrangianfluidsimulation_MainActivity_setupGraphics(JNIEnv* env, jobject obj, jobject assetManager) {
        mainview = new Mainview(AAssetManager_fromJava(env, assetManager));
        mainview->setupGraphics();
        mainview->getTransforms().setAspectRatio(aspectRatio);

        init();
        eglContextManager->initContext();
        LOGI("native-lib", "Graphics setup complete");
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
        fileDescriptors.reserve(len);
        for (int i = 0; i < len; i++) {
            fileDescriptors.push_back(fds[i]);
        }

        env->ReleaseIntArrayElements(jfds, fds, 0);
        LOGI("native-lib", "File descriptors loaded");
        loadInitStep();
        LOGI("native-lib", "Initial step loaded");
    }

    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_MainActivity_createBuffers(JNIEnv *env, jobject thiz) {
        mainview->createVectorFieldBuffer(vectorFieldHandler->getOldVertices());
        mainview->createParticlesBuffer(particlesHandler->getParticlesPositions());
        mainview->createComputeBuffer(vectorFieldHandler->getOldVertices(), vectorFieldHandler->getNewVertices(), vectorFieldHandler->getFutureVertices());
        mainview->loadConstUniforms(physics->dt, vectorFieldHandler->getWidth(), vectorFieldHandler->getHeight(), vectorFieldHandler->getDepth());
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
        delete eglContextManager;
    }

    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_MainActivity_loadDeviceInfo(JNIEnv *env, jobject thiz, jdouble jaspectRatio) {
        aspectRatio = (float) jaspectRatio;
    }

    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_FileAccessHelper_loadInitialPositions(JNIEnv *env, jobject thiz, jint fd) {
        if (particlesHandler->areParticlesInitialized()) {
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
