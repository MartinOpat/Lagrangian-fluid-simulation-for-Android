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



std::vector<float> vertices;
std::vector<std::vector<float>> allVertices;
std::vector<std::vector<float>> displayVertices;
int currentFrame = 0;
int numVertices = 0;
int width = 0;
int height = 0;
int depth = 0;
int fineness = 15;
bool started = false;

GLShaderManager* shaderManager;
ParticlesHandler* particlesHandler;

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

void velocityField(Point position, Vec3& velocity) {
    int fineness = 1;  // TODO: Remove once definitely not needed
    int adjWidth = width / fineness;
    int adjHeight = height / fineness;
    int adjDepth = depth / (fineness / 2);

    // Transform position [-1, 1] range to [0, adjWidth/adjHeight] grid indices
    int gridX = (int)((position.x + 1.0) / 2 * adjWidth);
    int gridY = (int)((position.y + 1.0) / 2 * adjHeight);
    int gridZ = (int)((position.z + 1.0) / 2 * adjDepth);
//    int gridZ = abs((int)(position.z * depth));

    // Ensure indices are within bounds
    gridX = std::max(0, std::min(gridX, adjWidth - 1));
    gridY = std::max(0, std::min(gridY, adjHeight - 1));
    gridZ = std::max(0, std::min(gridZ, adjHeight - 1));

    int idx = gridZ* adjWidth * adjHeight + gridY * adjWidth + gridX;

    // Calculate velocity as differences
    velocity = Vec3(allVertices[currentFrame][idx * 6 + 3] - allVertices[currentFrame][idx * 6],
                    allVertices[currentFrame][idx * 6 + 4] - allVertices[currentFrame][idx * 6 + 1],
                    allVertices[currentFrame][idx * 6 + 5] - allVertices[currentFrame][idx * 6 + 2]
                    );
}

void prepareVertexData(const std::vector<float>& uData, const std::vector<float>& vData, int width, int height) {

    vertices.clear();
    std::vector<float> tempVertices;

    float maxU = *std::max_element(uData.begin(), uData.end());
    float minU = *std::min_element(uData.begin(), uData.end());
    float maxV = *std::max_element(vData.begin(), vData.end());
    float minV = *std::min_element(vData.begin(), vData.end());

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = y * width + x;

            float normalizedX = (x / (float)(width)) * 2 - 1;
            float normalizedY = (y / (float)(height)) * 2 - 1;

            float scaleFactor = 0.1f;
            float normalizedU = 2 * ((uData[index] - minU) / (maxU - minU)) - 1;
            normalizedU *= scaleFactor;
            float normalizedV = 2 * ((vData[index] - minV) / (maxV - minV)) - 1;
            normalizedV *= scaleFactor;

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

            if (y % fineness != 0 || x % fineness != 0) continue;
            tempVertices.push_back(normalizedX);
            tempVertices.push_back(normalizedY);
            tempVertices.push_back(0.0f);

            tempVertices.push_back(endX);
            tempVertices.push_back(endY);
            tempVertices.push_back(0.0f);
        }
    }
    numVertices = vertices.size();
    allVertices.push_back(vertices);
    displayVertices.push_back(tempVertices);
}

void prepareVertexData(const std::vector<float>& uData, const std::vector<float>& vData, const std::vector<float>& wData, int width, int height, int depth) {

    vertices.clear();
    std::vector<float> tempVertices;

    LOGI("3d");

    float maxU = *std::max_element(uData.begin(), uData.end());
    float minU = *std::min_element(uData.begin(), uData.end());
    float maxV = *std::max_element(vData.begin(), vData.end());
    float minV = *std::min_element(vData.begin(), vData.end());
    float maxW = *std::max_element(wData.begin(), wData.end());
    float minW = *std::min_element(wData.begin(), wData.end());

//    LOGI("Max W: %f, Min W: %f", maxW, minW);
    for (int z = 0; z < depth; z++) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {

                int index = z * width * height + y * width + x;

                float normalizedX = (x / (float)(width)) * 2 - 1;
                float normalizedY = (y / (float)(height)) * 2 - 1;
                float normalizedZ = (z / (float)(depth)) * 2 - 1;
//                float normalizedZ = z;

                float scaleFactor = 0.1f;
                float normalizedU = 2 * ((uData[index] - minU) / (maxU - minU)) - 1;
                normalizedU *= scaleFactor;
                float normalizedV = 2 * ((vData[index] - minV) / (maxV - minV)) - 1;
                normalizedV *= scaleFactor;
                float normalizedW = 2 * ((wData[index] - minW) / (maxW - minW)) - 1;
                normalizedW *= scaleFactor;

                float endX = normalizedX + normalizedU;
                float endY = normalizedY + normalizedV;
                float endZ = normalizedZ + normalizedW;

                // Start point
                vertices.push_back(normalizedX);
                vertices.push_back(normalizedY);
                vertices.push_back(normalizedZ);

                // End point
                vertices.push_back(endX);
                vertices.push_back(endY);
                vertices.push_back(endZ);

                if (z % (fineness / 2) != 0 || y % fineness != 0 || x % fineness != 0) continue;
                tempVertices.push_back(normalizedX);
                tempVertices.push_back(normalizedY);
                tempVertices.push_back(normalizedZ);

                tempVertices.push_back(endX);
                tempVertices.push_back(endY);
                tempVertices.push_back(endZ);
            }
        }
    }
    numVertices = vertices.size();
    allVertices.push_back(vertices);
    displayVertices.push_back(tempVertices);
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
        width = countp[3];
        height = countp[2];
        depth = 1;
        prepareVertexData(uData, vData, countp[3], countp[2]);
    }
}

void loadAllTimeSteps(const std::string& fileUPath, const std::string& fileVPath, const std::string& fileWPath) {
    netCDF::NcFile dataFileU(fileUPath, netCDF::NcFile::read);
    netCDF::NcFile dataFileV(fileVPath, netCDF::NcFile::read);
    netCDF::NcFile dataFileW(fileWPath, netCDF::NcFile::read);

    LOGI("NetCDF files opened");

    size_t numTimeSteps = dataFileU.getDim("time").getSize();

    for (size_t i = 0; i < 1; i++) {
        std::vector<size_t> startp = {i, 1, 0, 0};  // Start index for time, depth, y, x
        std::vector<size_t> countp = {1, dataFileU.getDim("depth").getSize()-1, dataFileU.getDim("lat").getSize(), dataFileU.getDim("lon").getSize()};  // Read one time step, all depths, all y, all x
        std::vector<float> uData( countp[1] * countp[2] * countp[3]), vData(countp[1] * countp[2] * countp[3]), wData(countp[1] * countp[2] * countp[3]);

        // Prepare vertex data for OpenGL from uData and vData, and store in allVertices[i]
        width = countp[3];
        height = countp[2];
        depth = countp[1];

        dataFileU.getVar("u").getVar(startp, countp, uData.data());
//        dataFileU.getVar("vozocrtx").getVar(startp, countp, uData.data());
        dataFileV.getVar("v").getVar(startp, countp, vData.data());
//        dataFileV.getVar("vomecrty").getVar(startp, countp, vData.data());
        dataFileW.getVar("w").getVar(startp, countp, wData.data());
//        dataFileW.getVar("W").getVar(startp, countp, wData.data());

        LOGI("Data loaded with width: %d, height: %d, depth: %d", width, height, depth);
        prepareVertexData(uData, vData, wData, width, height, depth);
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

        shaderManager->loadVectorFieldData(displayVertices[currentFrame]);
        shaderManager->drawVectorField(displayVertices[currentFrame].size());

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

        loadAllTimeSteps(tempFileU, tempFileV);
        LOGI("NetCDF files loaded of width: %d, height: %d", width, height);

        particlesHandler = new ParticlesHandler(ParticlesHandler::InitType::two_lines, velocityField);
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

        loadAllTimeSteps(tempFileU, tempFileV, tempFileW);
        LOGI("NetCDF files loaded");

        particlesHandler = new ParticlesHandler(ParticlesHandler::InitType::line, velocityField);
        LOGI("Particles initialized");
    }

    JNIEXPORT void JNICALL
    Java_com_example_lagrangianfluidsimulation_MainActivity_createBuffers(JNIEnv *env, jobject thiz) {
        shaderManager->createVectorFieldBuffer(allVertices[currentFrame]);
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
