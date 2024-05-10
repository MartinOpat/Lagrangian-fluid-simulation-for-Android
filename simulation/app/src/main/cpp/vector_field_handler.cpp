//
// Created by martin on 08-05-2024.
//

#include "vector_field_handler.h"
#include "android_logging.h"
#include "netcdf_reader.h"
#include "consts.h"

VectorFieldHandler::VectorFieldHandler(int fineness): fineness(fineness) {}

void VectorFieldHandler::velocityField(const glm::vec3 &position, glm::vec3 &velocity) {
    int fineness = 1;  // TODO: Remove once definitely not needed
    int adjWidth = width / fineness;
    int adjHeight = height / fineness;
    int adjDepth = 0;

    // Transform position [-1, 1] range to [0, adjWidth/adjHeight] grid indices
    int gridX = (int)((position.x / 100.0f + 1.0) / 2 * adjWidth);
    int gridY = (int)((position.y / 100.0f + 1.0) / 2 * adjHeight);
    int gridZ = (int)((position.z / 100.0f + 1.0) / 2 * adjDepth);
//    int gridZ = abs((int)(position.z * depth));

    // Ensure indices are within bounds
    gridX = std::max(0, std::min(gridX, adjWidth - 1));
    gridY = std::max(0, std::min(gridY, adjHeight - 1));
    gridZ = std::max(0, std::min(gridZ, adjHeight - 1));

    int idx = gridZ* adjWidth * adjHeight + gridY * adjWidth + gridX;

    velocity = glm::vec3(allVertices[currentFrame][idx * 6 + 3] - allVertices[currentFrame][idx * 6],
                        allVertices[currentFrame][idx * 6 + 4] - allVertices[currentFrame][idx * 6 + 1],
                        allVertices[currentFrame][idx * 6 + 5] - allVertices[currentFrame][idx * 6 + 2]
    );
}

void VectorFieldHandler::prepareVertexData(const std::vector<float>& uData, const std::vector<float>& vData) {

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

void VectorFieldHandler::prepareVertexData(const std::vector<float>& uData, const std::vector<float>& vData, const std::vector<float>& wData) {

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

                float normalizedX = FIELD_WIDTH*((x / (float)(width)) * 2 - 1);
//                float scaledX = x;
                float normalizedY = FIELD_HEIGHT*((y / (float)(height)) * 2 - 1);
//                float scaledY = y;
                float normalizedZ = FIELD_DEPTH*((z / (float)(depth)) * 2 - 1);
//                float scaledZ = z;
//                float normalizedZ = z;

                float scaleFactor = 10.0f;
                float normalizedU = 2 * ((uData[index] - minU) / (maxU - minU)) - 1;
                normalizedU *= scaleFactor;
                float normalizedV = 2 * ((vData[index] - minV) / (maxV - minV)) - 1;
                normalizedV *= scaleFactor;
                float normalizedW = 2 * ((wData[index] - minW) / (maxW - minW)) - 1;
                normalizedW *= scaleFactor;

                float endX = normalizedX + normalizedU;
//                float endX = scaledX + uData[index];
                float endY = normalizedY + normalizedV;
//                float endY = scaledY + vData[index];
                float endZ = normalizedZ + normalizedW;
//                float endZ = scaledZ + wData[index];

                // Start point
                vertices.push_back(normalizedX);
//                vertices.push_back(scaledX);
                vertices.push_back(normalizedY);
//                vertices.push_back(scaledY);
                vertices.push_back(normalizedZ);
//                vertices.push_back(scaledZ);

                // End point
                vertices.push_back(endX);
                vertices.push_back(endY);
                vertices.push_back(endZ);

                if (z % (fineness / 2) != 0 || y % fineness != 0 || x % fineness != 0) continue;
                tempVertices.push_back(normalizedX);
//                tempVertices.push_back(scaledX);
                tempVertices.push_back(normalizedY);
//                tempVertices.push_back(scaledY);
                tempVertices.push_back(normalizedZ);
//                tempVertices.push_back(scaledZ);


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

void VectorFieldHandler::loadAllTimeSteps(const std::string& fileUPath, const std::string& fileVPath) {
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
        LOGI("Data loaded with width: %d, height: %d, depth: %d", width, height, depth);
        prepareVertexData(uData, vData);
    }
}

void VectorFieldHandler::loadAllTimeSteps(const std::string& fileUPath, const std::string& fileVPath, const std::string& fileWPath) {
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
        dataFileV.getVar("v").getVar(startp, countp, vData.data());
        dataFileW.getVar("w").getVar(startp, countp, wData.data());

        LOGI("Data loaded with width: %d, height: %d, depth: %d", width, height, depth);
        prepareVertexData(uData, vData, wData);
    }
}

void VectorFieldHandler::draw(GLShaderManager& shaderManager) {
    shaderManager.loadVectorFieldData(displayVertices[currentFrame]);
    shaderManager.drawVectorField(displayVertices[currentFrame].size());
}