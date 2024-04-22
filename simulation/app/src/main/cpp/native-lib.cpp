#include <jni.h>
#include <string>
#include <GLES2/gl2.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <chrono>
#include <netcdf>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>

#include "mainview.h"

#define LOG_TAG "native-lib"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

std::vector<float> vertices;
std::vector<std::vector<float>> allVertices;
int currentFrame = 0;
int numVertices = 0;

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

std::string writeTempFileFromFD(int fd, const std::string& tempFilename) {
    // Generate path for the temporary file in the app's internal storage
    std::string tempFilePath = "/data/data/com.example.lagrangianfluidsimulation/tmp/" + tempFilename;

    // Ensure the directory exists
    mkdir("/data/data/com.example.lagrangianfluidsimulation/tmp/", 0777);

    // Create and open the temporary file
    int tempFd = open(tempFilePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (tempFd == -1) {
        __android_log_print(ANDROID_LOG_ERROR, "native-lib", "Failed to open temporary file for writing");
        return "";
    }

    // Rewind the source descriptor to ensure it's read from the start
    lseek(fd, 0, SEEK_SET);

    // Copy data from the file descriptor to the temporary file
    char buffer[1024];
    ssize_t bytesRead;
    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        if (write(tempFd, buffer, bytesRead) != bytesRead) {
            __android_log_print(ANDROID_LOG_ERROR, "native-lib", "Failed to write all bytes to temporary file");
            close(tempFd);
            return "";
        }
    }

    close(tempFd);
    return tempFilePath;
}

void prepareVertexData(const std::vector<float>& uData, const std::vector<float>& vData, int width, int height) {

    vertices.clear();

    float maxU = *std::max_element(uData.begin(), uData.end());
    float minU = *std::min_element(uData.begin(), uData.end());
    float maxV = *std::max_element(vData.begin(), vData.end());
    float minV = *std::min_element(vData.begin(), vData.end());

    for (int y = 0; y < height; y++) {
        if (y % 25 != 0) continue;
        for (int x = 0; x < width; x++) {
            int index = y * width + x;

            if (x % 25 != 0) continue;

            float normalizedX = (x / (float)(width - 1)) * 2 - 1;
            float normalizedY = (y / (float)(height - 1)) * 2 - 1;

            float normalizedU = 2 * ((uData[index] - minU) / (maxU - minU)) - 1;
            normalizedU *= 0.1f;
            float normalizedV = 2 * ((vData[index] - minV) / (maxV - minV)) - 1;
            normalizedV *= 0.1f;

            float endX = normalizedX + normalizedU;
            float endY = normalizedY + normalizedV;

            // Start point
            vertices.push_back(normalizedX);
            vertices.push_back(normalizedY);
            vertices.push_back(0.0f);

            // End point
            vertices.push_back(endX);
            vertices.push_back(endY);
            vertices.push_back(0.0f);
        }
    }
    numVertices = vertices.size();
    allVertices.push_back(vertices);
}

void loadAllTimeSteps(const std::string& fileUPath, const std::string& fileVPath) {
    netCDF::NcFile dataFileU(fileUPath, netCDF::NcFile::read);
    netCDF::NcFile dataFileV(fileVPath, netCDF::NcFile::read);

    LOGI("NetCDF files opened");

    size_t numTimeSteps = dataFileU.getDim("time_counter").getSize();

    for (size_t i = 0; i < 1; i++) {
        std::vector<size_t> startp = {i, 0, 0, 0};  // Start index for time, depth, y, x
        std::vector<size_t> countp = {1, 1, dataFileU.getDim("y").getSize(), dataFileU.getDim("x").getSize()};  // Read one time step, all y, all x
        std::vector<float> uData(countp[2] * countp[3]), vData(countp[2] * countp[3]);

        dataFileU.getVar("vozocrtx").getVar(startp, countp, uData.data());
        dataFileV.getVar("vomecrty").getVar(startp, countp, vData.data());

        // Prepare vertex data for OpenGL from uData and vData, and store in allVertices[i]
        prepareVertexData(uData, vData, countp[3], countp[2]);
    }
}

void updateFrame() {
    static auto lastUpdate = std::chrono::steady_clock::now(); // Last update time
    static const std::chrono::seconds updateInterval(1);       // Update every 1 second

    auto now = std::chrono::steady_clock::now();
    if (now - lastUpdate >= updateInterval) {                  // Check if 1 second has passed
        currentFrame = (currentFrame + 1) % allVertices.size(); // Update the frame index
        lastUpdate = now;                                      // Reset the last update time
    }
}


extern "C" {
    JNIEXPORT void JNICALL Java_com_example_lagrangianfluidsimulation_MainActivity_drawFrame(JNIEnv* env, jobject /* this */) {
        shaderManager->setFrame();
//        shaderManager->drawTriangle();
        setParticlePosition();
        shaderManager->drawParticle();

        shaderManager->loadVectorFieldData(allVertices[currentFrame]);
        shaderManager->drawVectorField(numVertices);
//        updateFrame();
//        print_nc_vars("/home/martin/Lagrangian-fluid-simulation-for-Android/simulation/data/DoubleGyre2D/doublegyreU.nc");
    }

    JNIEXPORT void JNICALL Java_com_example_lagrangianfluidsimulation_MainActivity_setupGraphics(JNIEnv* env, jobject obj, jobject assetManager) {
        shaderManager = new GLShaderManager(AAssetManager_fromJava(env, assetManager));
        shaderManager->setupGraphics();
        LOGI("Graphics setup complete");
//        print_nc_vars_from_asset(AAssetManager_fromJava(env, assetManager), "test_data/doublegyreU.nc");
    }

    JNIEXPORT void JNICALL
    Java_com_example_lagrangianfluidsimulation_MainActivity_initializeNetCDFVisualization(
            JNIEnv* env, jobject /* this */, jint fdU, jint fdV) {

        LOGI("Initializing NetCDF visualization");
        std::string tempFileU = writeTempFileFromFD(fdU, "tempU.nc");
        std::string tempFileV = writeTempFileFromFD(fdV, "tempV.nc");

        if (tempFileU.empty() || tempFileV.empty()) {
            LOGE("Failed to create temporary files.");
            return;
        }

//        LOGI("Temporary files created: %s, %s", tempFileU.c_str(), tempFileV.c_str());
//
//        // Create NetCDF file handlers
//        netCDF::NcFile dataFileU(tempFileU, netCDF::NcFile::read);
//        netCDF::NcFile dataFileV(tempFileV, netCDF::NcFile::read);
//
//        LOGI("NetCDF files opened");
//
//        // Open the U component file
//        netCDF::NcVar dataVarU = dataFileU.getVar("vozocrtx");
//
//        assert(!dataVarU.isNull());
//        // Assume 3D data: time, Y, X
//        std::vector<size_t> startp = {0, 0, 0, 0}; // Start at the first time step
//        std::vector<size_t> countp = {1,  1, dataVarU.getDim(2).getSize(), dataVarU.getDim(3).getSize()}; // One time step, one depth, all Y, all X
//
//        std::vector<float> uData(dataVarU.getDim(2).getSize() * dataVarU.getDim(3).getSize());
//        dataVarU.getVar(startp, countp, uData.data());
//
//        // Open the V component file
//        netCDF::NcVar dataVarV = dataFileV.getVar("vomecrty");
//
//        assert(!dataVarV.isNull());
//        // Assume 3D data: time, Y, X
//        startp = {0, 0, 0, 0}; // Start at the first time step
//        countp = {1, 1, dataVarV.getDim(2).getSize(), dataVarV.getDim(3).getSize()}; // One time step, one depth, all Y, all X
//
//        std::vector<float> vData(dataVarV.getDim(2).getSize() * dataVarV.getDim(3).getSize());
//        dataVarV.getVar(startp, countp, vData.data());
//
//        // After loading data, prepare vertex data for OpenGL
//        int width = dataVarU.getDim(3).getSize(); // Assuming dim(2) is 'x'
//        int height = dataVarU.getDim(2).getSize(); // Assuming dim(1) is 'y'
//        prepareVertexData(uData, vData, width, height);

        LOGI("Loading time steps");
        loadAllTimeSteps(tempFileU, tempFileV);
        LOGI("Time steps loaded");

    }

    JNIEXPORT void JNICALL
    Java_com_example_lagrangianfluidsimulation_MainActivity_createBuffers(JNIEnv *env, jobject thiz) {
        shaderManager->createVectorFieldBuffer(allVertices[currentFrame]);
    }
} // extern "C"
