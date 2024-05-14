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
std::vector<int> fileDescriptors;

GLShaderManager* shaderManager;
ParticlesHandler* particlesHandler;
VectorFieldHandler* vectorFieldHandler;
TouchHandler* touchHandler;
Physics* physics;


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

void init() {
    vectorFieldHandler = new VectorFieldHandler();
    LOGI("NetCDF files loaded");

    physics = new Physics(*vectorFieldHandler, Physics::Model::particles_advection);

    particlesHandler = new ParticlesHandler(ParticlesHandler::InitType::line, *physics);
    LOGI("Particles initialized");

    LOGI("Init called");
}

extern "C" {
    JNIEXPORT void JNICALL Java_com_example_lagrangianfluidsimulation_MainActivity_drawFrame(JNIEnv* env, jobject /* this */) {
        shaderManager->setFrame();
        vectorFieldHandler->draw(*shaderManager);

        particlesHandler->drawParticles(*shaderManager);

        //        updateFrame();
        frameCount++;
    }

    JNIEXPORT void JNICALL Java_com_example_lagrangianfluidsimulation_MainActivity_setupGraphics(JNIEnv* env, jobject obj, jobject assetManager) {
        shaderManager = new GLShaderManager(AAssetManager_fromJava(env, assetManager));
        shaderManager->setupGraphics();

        touchHandler = new TouchHandler(*shaderManager);
        LOGI("Graphics setup complete");

        init();  // TODO: Think whether this belongs here or should be its own native func.
        LOGI("Setup complete");
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

//        vectorFieldHandler = new VectorFieldHandler();
        vectorFieldHandler->loadAllTimeSteps(tempFileU, tempFileV);

//        physics = new Physics(*vectorFieldHandler);

//        particlesHandler = new ParticlesHandler(ParticlesHandler::InitType::line, *physics);
        LOGI("Particles initialized");
    }

    JNIEXPORT void JNICALL
        Java_com_example_lagrangianfluidsimulation_FileAccessHelper_loadFilesFDs(
                JNIEnv* env, jobject /* this */, jintArray jfds) {
        jsize len = env->GetArrayLength(jfds);
        jint* fds = env->GetIntArrayElements(jfds, nullptr);
        fileDescriptors.clear();
        for (int i = 0; i < len; i++) {
            fileDescriptors.push_back(fds[i]);
        }

        env->ReleaseIntArrayElements(jfds, fds, 0);
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
//
        physics = new Physics(*vectorFieldHandler, Physics::Model::particles_advection);
//
        particlesHandler = new ParticlesHandler(ParticlesHandler::InitType::line, *physics);
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
