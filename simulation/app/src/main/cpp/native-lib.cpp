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

    JNIEXPORT void JNICALL Java_com_example_lagrangianfluidsimulation_MainActivity_temp(JNIEnv* env, jobject obj, jobject assetManager, jstring assetName) {
        const char *cAssetName = env->GetStringUTFChars(assetName, nullptr);
        AAssetManager *mgr = AAssetManager_fromJava(env, assetManager);
        if (mgr == nullptr) {
            LOGE("Error loading asset manager");
            return;
        }

        AAsset *asset = AAssetManager_open(mgr, cAssetName, AASSET_MODE_BUFFER);
        if (asset == nullptr) {
            LOGE("Error opening asset: %s", cAssetName);
            env->ReleaseStringUTFChars(assetName, cAssetName);
            return;
        }

        // Create a temporary file where you can write the asset contents
        std::string tempFileName = "/data/data/com.example.lagrangianfluidsimulation/cache/tempfile.nc"; // Adjust the path according to your package name
        FILE *out = fopen(tempFileName.c_str(), "wb");
        if (out == nullptr) {
            LOGE("Failed to open temporary file for writing");
            AAsset_close(asset);
            env->ReleaseStringUTFChars(assetName, cAssetName);
            return;
        }

        // Read the asset content and write it to the temporary file
        const size_t bufSize = 1024;
        char buf[bufSize];
        int bytesRead;
        while ((bytesRead = AAsset_read(asset, buf, bufSize)) > 0) {
            fwrite(buf, sizeof(char), bytesRead, out);
        }

        fclose(out);
        AAsset_close(asset);
        env->ReleaseStringUTFChars(assetName, cAssetName);

        netCDF::NcFile dataFile(tempFileName.c_str(), netCDF::NcFile::read);
        netCDF::NcVar vozocrtxVar = dataFile.getVar("x");

        if (!vozocrtxVar.isNull()) {
            // Read a small part of the data for printing
            // For example, first time step, first depth layer, and a 5x5 section from the top-left corner
            std::vector<size_t> startp = {0, 0, 0, 0}; // time, depth, y, x start indices
            std::vector<size_t> countp = {1, 1, 5, 5}; // time, depth, y, x count sizes

            std::vector<float> data(5 * 5); // Buffer to hold the data
            vozocrtxVar.getVar(startp, countp, data.data());

            // Print the data
            for (int i = 0; i < 5; ++i) { // Rows
                for (int j = 0; j < 5; ++j) { // Columns
//                    std::cout << data[i * 5 + j] << " ";
                    LOGI("%f", data[i * 5 + j]);
                }
//                std::cout << std::endl;
                LOGI("\n");
            }
        } else {
            LOGE("Variable vozocrtx not found in the file");
        }

        // Now that the file is written, you can open it with your netCDF reader
//        print_nc_vars(tempFileName.c_str());
    }
} // extern "C"
