#include <jni.h>
#include <string>
#include <GLES2/gl2.h>
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



std::vector<float> vertices;
std::vector<std::vector<float>> allVertices;
int currentFrame = 0;
int numVertices = 0;
int width = 0;
int height = 0;
int fineness = 15;
bool started = false;

float b = 0.8f;  // Drag coefficient
std::vector<Particle> particles;
std::vector<float> particlesPos;

int frameCount = 0;
float timeCount = 0.0f;


GLShaderManager* shaderManager;

void updateParticlePosArr() {
    particlesPos.clear();
    for (auto& particle : particles) {
        Vec3 particlePos = particle.getPosition();
        particlesPos.push_back(particlePos.x);
        particlesPos.push_back(particlePos.y);
        particlesPos.push_back(particlePos.z);
    }
}

void initParticles(int num) {
    particles.clear();
    particlesPos.clear();
    for (int i = 0; i < num; i++) {
        // Randomly generate initial velocity
        float aspectRatio = 19.3f / 9.0f;
        float angle = 2.0f * M_PI * rand() / (float)RAND_MAX;
        float magnitude = 0.3f * rand() / (float)RAND_MAX;
        float xVel = magnitude * cos(angle) * aspectRatio;
        float yVel = magnitude * sin(angle);
        Vec3 initialVel(xVel, yVel, 0.0f);
        Vec3 initialPos(-0.25f, 0.25f, 0.0f);
        particles.push_back(Particle(initialPos, initialVel));
    }

    updateParticlePosArr();
}

void velocityField(Point position, Vec3& velocity) {
    int adjWidth = width / fineness;
    int adjHeight = height / fineness;

    // Transform position [-1, 1] range to [0, adjWidth/adjHeight] grid indices
    int gridX = (int)((position.x + 1.0) / 2 * adjWidth);
    int gridY = (int)((position.y + 1.0) / 2 * adjHeight);
//    Point gridPosition = ((position + 1.0) / 2) * Point(adjWidth, adjHeight, 0);

    // Ensure indices are within bounds
    gridX = std::max(0, std::min(gridX, adjWidth - 1));
    gridY = std::max(0, std::min(gridY, adjHeight - 1));

    int idx = gridY * adjWidth + gridX;

    // Calculate velocity as differences
    velocity = Vec3(allVertices[currentFrame][idx * 6 + 3] - allVertices[currentFrame][idx * 6],
                    allVertices[currentFrame][idx * 6 + 4] - allVertices[currentFrame][idx * 6 + 1], 0);
}

void updateParticles() {
    if (!started) {
        shaderManager->startTime = std::chrono::steady_clock::now();
        started = true;
    }
    auto currentTime = std::chrono::steady_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - shaderManager->startTime).count();
    shaderManager->startTime = currentTime;

    timeCount += deltaTime;
    if (timeCount >= 1.0f) {
        LOGI("Frame rate: %d", frameCount);
        frameCount = 0;
        timeCount = 0.0f;
    }

    for (auto& particle : particles) {
        particle.rk4Step(deltaTime, velocityField, b);
    }
}

void prepareVertexData(const std::vector<float>& uData, const std::vector<float>& vData, int width, int height) {

    vertices.clear();

    float maxU = *std::max_element(uData.begin(), uData.end());
    float minU = *std::min_element(uData.begin(), uData.end());
    float maxV = *std::max_element(vData.begin(), vData.end());
    float minV = *std::min_element(vData.begin(), vData.end());

    for (int y = 0; y < height; y++) {
        if (y % fineness != 0) continue;
        for (int x = 0; x < width; x++) {
            int index = y * width + x;

            if (x % fineness != 0) continue;

            float normalizedX = (x / (float)(width)) * 2 - 1;
            float normalizedY = (y / (float)(height)) * 2 - 1;

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
        width = countp[3];
        height = countp[2];
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

        updateParticles();
        updateParticlePosArr();
        shaderManager->loadParticlesData(particlesPos);
        shaderManager->drawParticles(particlesPos.size());

        shaderManager->loadVectorFieldData(allVertices[currentFrame]);
        shaderManager->drawVectorField(numVertices);

        frameCount++;
//        updateFrame();
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
        LOGI("NetCDF files loaded");

        initParticles(1000);
        LOGI("Particles initialized");
    }

    JNIEXPORT void JNICALL
    Java_com_example_lagrangianfluidsimulation_MainActivity_createBuffers(JNIEnv *env, jobject thiz) {
        shaderManager->createVectorFieldBuffer(allVertices[currentFrame]);
        shaderManager->createParticlesBuffer(particlesPos);
        LOGI("Buffers created");
    }
} // extern "C"
