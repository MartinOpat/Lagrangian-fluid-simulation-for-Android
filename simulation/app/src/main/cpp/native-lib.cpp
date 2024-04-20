#include <jni.h>
#include <string>
#include <GLES2/gl2.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <chrono>
#include <netcdf>

#include "mainview.h"
#include "test_read_nc.h"

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
} // extern "C"
