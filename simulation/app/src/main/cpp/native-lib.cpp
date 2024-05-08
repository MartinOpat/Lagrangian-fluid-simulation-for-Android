#include <jni.h>
#include <string>
#include <GLES3/gl3.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <chrono>
#include <netcdf>
#include <assert.h>

#include "android_logging.h"
#include "netcdf_reader.h"
#include "mainview.h"
#include "triple.h"
#include "particle.h"
#include "particles_handler.h"
#include "vector_field_handler.h"


bool started = false;

GLShaderManager* shaderManager;
ParticlesHandler* particlesHandler;
VectorFieldHandler* vectorFieldHandler;

struct TouchPoint {
    float startX;
    float startY;
    float currentX;
    float currentY;
};

TouchPoint tpScale1;
TouchPoint tpScale2;
float prevScale = 0.5f;

TouchPoint tpRot;
Vec3 prevRot(0.0f, 0.0f, 0.0f);


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


extern "C" {
    JNIEXPORT void JNICALL Java_com_example_lagrangianfluidsimulation_MainActivity_drawFrame(JNIEnv* env, jobject /* this */) {
        shaderManager->setFrame();

//        shaderManager->loadVectorFieldData(displayVertices[currentFrame]);
//        shaderManager->drawVectorField(displayVertices[currentFrame].size());
        vectorFieldHandler->draw(*shaderManager);

        particlesHandler->drawParticles(*shaderManager);

        //        updateFrame();
        frameCount++;
    }

    JNIEXPORT void JNICALL Java_com_example_lagrangianfluidsimulation_MainActivity_setupGraphics(JNIEnv* env, jobject obj, jobject assetManager) {
        shaderManager = new GLShaderManager(AAssetManager_fromJava(env, assetManager));
        shaderManager->setupGraphics();
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
//        shaderManager->createVectorFieldBuffer(allVertices[currentFrame]);
        shaderManager->createVectorFieldBuffer(vectorFieldHandler->getAllVertices());
        shaderManager->createParticlesBuffer(particlesHandler->getParticlesPositions());
        LOGI("Buffers created");
    }

    JNIEXPORT void JNICALL
    Java_com_example_lagrangianfluidsimulation_MainActivity_nativeSendTouchEvent(JNIEnv *env, jobject obj, jint pointerCount, jfloatArray xArray, jfloatArray yArray, jint action) {
        jfloat* x = env->GetFloatArrayElements(xArray, nullptr);
        jfloat* y = env->GetFloatArrayElements(yArray, nullptr);


        // 0, 5 -> click
        // 1, 6 -> release
        // 2 -> move

        LOGI("Touch event: %d", action);
        if (pointerCount == 1) {
            if (action == 0) {
                tpRot.startX = x[0];
                tpRot.startY = y[0];
                tpRot.currentX = x[0];
                tpRot.currentY = y[0];
                prevRot = shaderManager->getRotation();
            } else if (action == 1) {
                tpRot.startX = 0.0f;
                tpRot.startY = 0.0f;
                tpRot.currentX = 0.0f;
                tpRot.currentY = 0.0f;
            } else if (action == 2) {
                tpRot.currentX = x[0];
                tpRot.currentY = y[0];

                float rotSensitivity = 0.001f;
                float dx = tpRot.currentX - tpRot.startX;
                float dy = tpRot.currentY - tpRot.startY;
                shaderManager->setRotation(rotSensitivity*dy + prevRot.x, rotSensitivity*dx + prevRot.y, prevRot.z);
            }
        } else if (pointerCount == 2) {
            if (action == 5) {
                tpScale1.startX = x[0];
                tpScale1.startY = y[0];
                tpScale1.currentX = x[0];
                tpScale1.currentY = y[0];
                tpScale2.startX = x[1];
                tpScale2.startY = y[1];
                tpScale2.currentX = x[1];
                tpScale2.currentY = y[1];
                prevScale = shaderManager->getScale();
            } else if (action == 6) {
                tpScale1.startX = 0.0f;
                tpScale1.startY = 0.0f;
                tpScale1.currentX = 0.0f;
                tpScale1.currentY = 0.0f;
                tpScale2.startX = 0.0f;
                tpScale2.startY = 0.0f;
                tpScale2.currentX = 0.0f;
                tpScale2.currentY = 0.0f;
            } else if (action == 2) {
                tpScale1.currentX = x[0];
                tpScale1.currentY = y[0];

                tpScale2.currentX = x[1];
                tpScale2.currentY = y[1];

                float currDist = (tpScale1.currentX - tpScale2.currentX)*(tpScale1.currentX - tpScale2.currentX) +
                                      (tpScale1.currentY - tpScale2.currentY)*(tpScale1.currentY - tpScale2.currentY);
                float initDist = (tpScale1.startX - tpScale2.startX)*(tpScale1.startX - tpScale2.startX) +
                                      (tpScale1.startY - tpScale2.startY)*(tpScale1.startY - tpScale2.startY);
                float scale = sqrt(currDist / initDist);
                shaderManager->setScale(scale * prevScale);
            }
        }

        env->ReleaseFloatArrayElements(xArray, x, 0);
        env->ReleaseFloatArrayElements(yArray, y, 0);
    }
} // extern "C"
