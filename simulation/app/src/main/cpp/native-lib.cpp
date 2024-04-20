#include <jni.h>
#include <string>
#include <GLES2/gl2.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <chrono>
#include <netcdf>

#include "mainview.h"

#define LOG_TAG "native-lib"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)



struct Vec3 {
    float x, y, z;
};

Vec3 particlePosition = {0.0f, -0.25f, 0.0f}; // Initial position
Vec3 velocity = {0.1f, 0.0f, 0.0f}; // Speed and direction

GLShaderManager* shaderManager;

void updateParticlePosition(float deltaTime) {  // TODO: This should probably be done in the GPU (not CPU) cause SIMD, look into compute shaders
    // Update position based on velocity
    particlePosition.x += velocity.x * deltaTime;
    particlePosition.y += velocity.y * deltaTime;

    // Wrap the position around the screen
    if (particlePosition.x > 1.0f) particlePosition.x = -1.0f;
    else if (particlePosition.x < -1.0f) particlePosition.x = 1.0f;

    if (particlePosition.y > 1.0f) particlePosition.y = -1.0f;
    else if (particlePosition.y < -1.0f) particlePosition.y = 1.0f;
}

void setParticlePosition() {
    // Update deltaTime based on your application's timing logic
    auto currentTime = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - shaderManager->startTime).count();
    shaderManager->startTime = currentTime;

    updateParticlePosition(deltaTime);

    // Set uniform for updated position
    GLint posLocation = glGetUniformLocation(shaderManager->shaderProgram, "uPosition");
    glUniform3f(posLocation, particlePosition.x, particlePosition.y, particlePosition.z);
}



extern "C" {
    JNIEXPORT void JNICALL Java_com_example_lagrangianfluidsimulation_MainActivity_drawFrame(JNIEnv* env, jobject /* this */) {
        shaderManager->setFrame();
        shaderManager->drawTriangle();
        setParticlePosition();
        shaderManager->drawParticle();
//        print_nc_vars("/home/martin/Lagrangian-fluid-simulation-for-Android/simulation/data/DoubleGyre2D/doublegyreU.nc");
    }

    JNIEXPORT void JNICALL Java_com_example_lagrangianfluidsimulation_MainActivity_setupGraphics(JNIEnv* env, jobject obj, jobject assetManager) {
        shaderManager = new GLShaderManager(AAssetManager_fromJava(env, assetManager));
        shaderManager->setupGraphics();
//        print_nc_vars_from_asset(AAssetManager_fromJava(env, assetManager), "test_data/doublegyreU.nc");
    }

    JNIEXPORT void JNICALL
    Java_com_example_lagrangianfluidsimulation_MainActivity_initializeNetCDFVisualization(
            JNIEnv* env, jobject obj, jstring fileUPath, jstring fileVPath) {

        const char* pathU = env->GetStringUTFChars(fileUPath, nullptr);
        const char* pathV = env->GetStringUTFChars(fileVPath, nullptr);

        try {
            // Open the U component file
            netCDF::NcFile dataFileU(pathU, netCDF::NcFile::read);
            netCDF::NcVar dataVarU = dataFileU.getVar("vozocrtx");
            if (!dataVarU.isNull()) {
                // Assume 3D data: time, Y, X
                std::vector<size_t> startp = {0, 0, 0}; // Start at the first time step
                std::vector<size_t> countp = {1, dataVarU.getDim(1).getSize(), dataVarU.getDim(2).getSize()}; // One time step, all Y, all X

                std::vector<float> uData(dataVarU.getDim(1).getSize() * dataVarU.getDim(2).getSize());
                dataVarU.getVar(startp, countp, uData.data());
            }

            // Open the V component file
            netCDF::NcFile dataFileV(pathV, netCDF::NcFile::read);
            netCDF::NcVar dataVarV = dataFileV.getVar("vomecrty");
            if (!dataVarV.isNull()) {
                // Assume 3D data: time, Y, X
                std::vector<size_t> startp = {0, 0, 0}; // Start at the first time step
                std::vector<size_t> countp = {1, dataVarV.getDim(1).getSize(), dataVarV.getDim(2).getSize()}; // One time step, all Y, all X

                std::vector<float> vData(dataVarV.getDim(1).getSize() * dataVarV.getDim(2).getSize());
                dataVarV.getVar(startp, countp, vData.data());
            }

            // Prepare data for OpenGL
            // Example: Process uData and vData for visualization

        } catch (const netCDF::exceptions::NcException& e) {
            std::cerr << "NetCDF error: " << e.what() << std::endl;
        }

        env->ReleaseStringUTFChars(fileUPath, pathU);
        env->ReleaseStringUTFChars(fileVPath, pathV);
    }
} // extern "C"
