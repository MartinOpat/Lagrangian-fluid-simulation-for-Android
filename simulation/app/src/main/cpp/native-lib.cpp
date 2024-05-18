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


bool started = false;
std::vector<int> fileDescriptors;

Mainview* shaderManager;
ParticlesHandler* particlesHandler;
VectorFieldHandler* vectorFieldHandler;
TouchHandler* touchHandler;
Physics* physics;

int currentFrame = 0;
int numFrames = 0;

int fps = 0;

float global_time_in_step = 0.0f;

void loadStepHelper(int fdU, int fdV, int fdW) {
    NetCDFReader reader;
    std::string tempFileU = reader.writeTempFileFromFD(fdU, "tempU.nc");
    std::string tempFileV = reader.writeTempFileFromFD(fdV, "tempV.nc");
    std::string tempFileW = reader.writeTempFileFromFD(fdW, "tempW.nc");

    if (tempFileU.empty() || tempFileV.empty() || tempFileW.empty()) {
        LOGE("Failed to create temporary files.");
        return;
    }

    if (vectorFieldHandler == nullptr) {
        LOGE("Vector field handler not initialized");
    }
    vectorFieldHandler->loadTimeStep(tempFileU, tempFileV, tempFileW);
}

void loadStep(int frame) {
    LOGI("Loading step %d", frame);
    loadStepHelper(fileDescriptors[frame], fileDescriptors[numFrames + frame], fileDescriptors[2*numFrames + frame]);
}

void loadInitStep() {
    if (numFrames == 0) {
        LOGE("No frames loaded");
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
    static std::thread loadThread;
//    static std::atomic<bool> isLoading(false);

    static const std::chrono::seconds fpsUpdateInterval(1);
    static auto fpsLastUpdate = std::chrono::steady_clock::now();

    static auto lastCall = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    global_time_in_step += std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCall).count() / 1000.0f;
    lastCall = now;

    if (now - lastUpdate >= updateInterval) {
        lastUpdate = now;
        global_time_in_step = 0.0f;

        if (loadThread.joinable()) {
            LOGI("Joining thread");
            loadThread.join();
        }
        vectorFieldHandler->updateTimeStep();

        currentFrame = (currentFrame + 1) % numFrames;

//        isLoading.store(true);
        loadThread = std::thread([frame = currentFrame]() {
            loadStep(frame);  // Should be thread safe due to how vectorfieldhandler is implemented
        });
    }
    if (now - fpsLastUpdate >= fpsUpdateInterval) {
        fpsLastUpdate = now;
        LOGI("FPS: %d", fps);
        fps = 0;
    } else {
        fps++;
    }
}

void init() {
    vectorFieldHandler = new VectorFieldHandler();
    physics = new Physics(*vectorFieldHandler, Physics::Model::particles_advection);
    particlesHandler = new ParticlesHandler(ParticlesHandler::InitType::line, *physics, NUM_PARTICLES);
    shaderManager->loadParticlesData(particlesHandler->getParticlesPositions());

    LOGI("init complete");
}

extern "C" {
    JNIEXPORT void JNICALL Java_com_rug_lagrangianfluidsimulation_MainActivity_drawFrame(JNIEnv* env, jobject /* this */) {
        update();
        shaderManager->setFrame();
        vectorFieldHandler->draw(*shaderManager);

        shaderManager->dispatchComputeShader(physics->dt, global_time_in_step, vectorFieldHandler->getWidth(), vectorFieldHandler->getHeight(), vectorFieldHandler->getDepth());
        particlesHandler->drawParticles(*shaderManager);
    }

    JNIEXPORT void JNICALL Java_com_rug_lagrangianfluidsimulation_MainActivity_setupGraphics(JNIEnv* env, jobject obj, jobject assetManager) {
        shaderManager = new Mainview(AAssetManager_fromJava(env, assetManager));
        shaderManager->setupGraphics();

        touchHandler = new TouchHandler(*shaderManager);
        LOGI("Graphics setup complete");
    }

    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_FileAccessHelper_loadNetCDFData(
            JNIEnv* env, jobject /* this */, jint fdU, jint fdV) {

        NetCDFReader reader;
        std::string tempFileU = reader.writeTempFileFromFD(fdU, "tempU.nc");
        std::string tempFileV = reader.writeTempFileFromFD(fdV, "tempV.nc");

        if (tempFileU.empty() || tempFileV.empty()) {
            LOGE("Failed to create temporary files.");
            return;
        }

        vectorFieldHandler->loadTimeStep(tempFileU, tempFileV);

        LOGI("Particles initialized");
    }

    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_FileAccessHelper_loadFilesFDs(
                JNIEnv* env, jobject /* this */, jintArray jfds) {
        LOGI("Loading file descriptors");
        jsize len = env->GetArrayLength(jfds);
        numFrames = len / 3;
        LOGI("Number of frames: %d", numFrames);

        jint* fds = env->GetIntArrayElements(jfds, nullptr);
        fileDescriptors.clear();
        for (int i = 0; i < len; i++) {
            fileDescriptors.push_back(fds[i]);
        }

        env->ReleaseIntArrayElements(jfds, fds, 0);
        LOGI("File descriptors loaded");
        init();
        loadInitStep();
        LOGI("Initial step loaded");
    }


    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_FileAccessHelper_loadNetCDFData3D(
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
        vectorFieldHandler->loadTimeStep(tempFileU, tempFileV, tempFileW);
        LOGI("NetCDF files loaded");

        physics = new Physics(*vectorFieldHandler, Physics::Model::particles_advection);

        particlesHandler = new ParticlesHandler(ParticlesHandler::InitType::line, *physics);
        LOGI("Particles initialized");
    }

    JNIEXPORT void JNICALL
    Java_com_rug_lagrangianfluidsimulation_MainActivity_createBuffers(JNIEnv *env, jobject thiz) {
        shaderManager->createVectorFieldBuffer(vectorFieldHandler->getOldVertices());
        shaderManager->createParticlesBuffer(particlesHandler->getParticlesPositions());
        shaderManager->createComputeBuffer(vectorFieldHandler->getOldVertices());
        LOGI("Buffers created");
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

        LOGI("Touch event: %d", action);

        env->ReleaseFloatArrayElements(xArray, xTemp, 0);
        env->ReleaseFloatArrayElements(yArray, yTemp, 0);
    }
} // extern "C"
