//
// Created by martin on 08-05-2024.
//

#include "vector_field_handler.h"
#include "android_logging.h"
#include "netcdf_reader.h"
#include "consts.h"

VectorFieldHandler::VectorFieldHandler(int finenessXY, int finenessZ): finenessXY(finenessXY), finenessZ(finenessZ) {}

void VectorFieldHandler::velocityField(const glm::vec3 &position, glm::vec3 &velocity) {
    int fineness = 1;  // TODO: Remove once definitely not needed
    int adjWidth = width / fineness;
    int adjHeight = height / fineness;
    int adjDepth = 0;

    // Transform position [-1, 1] range to [0, adjWidth/adjHeight] grid indices
    int gridX = (int)((position.x / 100.0f + 1.0) / 2 * adjWidth);
    int gridY = (int)((position.y / 100.0f + 1.0) / 2 * adjHeight);
    int gridZ = (int)((position.z / 100.0f + 1.0) / 2 * adjDepth);

    // Ensure indices are within bounds
    gridX = std::max(0, std::min(gridX, adjWidth - 1));
    gridY = std::max(0, std::min(gridY, adjHeight - 1));
    gridZ = std::max(0, std::min(gridZ, adjHeight - 1));

    int idx = gridZ* adjWidth * adjHeight + gridY * adjWidth + gridX;

    float x0 = allVertices[0][idx * 6 + 3] - allVertices[0][idx * 6];
    float y0 = allVertices[0][idx * 6 + 4] - allVertices[0][idx * 6 + 1];
    float z0 = allVertices[0][idx * 6 + 5] - allVertices[0][idx * 6 + 2];

    float x1 = allVertices[1][idx * 6 + 3] - allVertices[1][idx * 6];
    float y1 = allVertices[1][idx * 6 + 4] - allVertices[1][idx * 6 + 1];
    float z1 = allVertices[1][idx * 6 + 5] - allVertices[1][idx * 6 + 2];

    velocity = glm::vec3(x0 + global_time_in_step / (float) TIME_STEP_IN_SECONDS * (x1 - x0),
                         y0 + global_time_in_step / (float) TIME_STEP_IN_SECONDS * (y1 - y0),
                         z0 + global_time_in_step / (float) TIME_STEP_IN_SECONDS * (z1 - z0)
    );
//    velocity = glm::vec3(allVertices[0][idx * 6 + 3] - allVertices[0][idx * 6],
//                        allVertices[0][idx * 6 + 4] - allVertices[0][idx * 6 + 1],
//                        allVertices[0][idx * 6 + 5] - allVertices[0][idx * 6 + 2]
//    );
}

void VectorFieldHandler::prepareVertexData(const std::vector<float>& uData, const std::vector<float>& vData) {

    std::vector<float> vertices;
    std::vector<float> tempDisplayVertices;

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

            if (y % finenessXY != 0 || x % finenessXY != 0) continue;
            tempDisplayVertices.push_back(normalizedX);
            tempDisplayVertices.push_back(normalizedY);
            tempDisplayVertices.push_back(0.0f);

            tempDisplayVertices.push_back(endX);
            tempDisplayVertices.push_back(endY);
            tempDisplayVertices.push_back(0.0f);
        }
    }
    allVertices.push_back(vertices);
    displayVertices.push_back(tempDisplayVertices);
}

void VectorFieldHandler::prepareVertexData(const std::vector<float>& uData, const std::vector<float>& vData, const std::vector<float>& wData) {

    std::vector<float> vertices;
    std::vector<float> tempDisplayVertices;

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
                float normalizedY = FIELD_HEIGHT*((y / (float)(height)) * 2 - 1);
                float normalizedZ = FIELD_DEPTH*((z / (float)(depth)) * 2 - 1);


                float scaleFactor = 10.0f;
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

                if (z % finenessZ != 0 || y % finenessXY != 0 || x % finenessXY != 0) continue;
                tempDisplayVertices.push_back(normalizedX);
                tempDisplayVertices.push_back(normalizedY);
                tempDisplayVertices.push_back(normalizedZ);


                tempDisplayVertices.push_back(endX);
                tempDisplayVertices.push_back(endY);
                tempDisplayVertices.push_back(endZ);
            }
        }
    }

    if (allVertices.size() == 3) {
        LOGI("Loading new vertices");
        allVertices[2] = vertices;
        displayVertices[2] = tempDisplayVertices;
    } else {
        LOGI("Vertices not yet filled, pushing");
        allVertices.push_back(vertices);
        displayVertices.push_back(tempDisplayVertices);
    }
}

void VectorFieldHandler::updateTimeStep() {
    if (allVertices.size() > 2) {
        allVertices[0] = std::move(allVertices[1]);
        displayVertices[0] = std::move(displayVertices[1]);
        allVertices[1] = std::move(allVertices[2]);
        displayVertices[1] = std::move(displayVertices[2]);
    } else if (allVertices.size() > 1) {
        allVertices[0] = std::move(allVertices[1]);
        displayVertices[0] = std::move(displayVertices[1]);
    }
}

void VectorFieldHandler::loadTimeStep(const std::string& fileUPath, const std::string& fileVPath) {
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

void VectorFieldHandler::loadTimeStep(const std::string& fileUPath, const std::string& fileVPath, const std::string& fileWPath) {
    netCDF::NcFile dataFileU(fileUPath, netCDF::NcFile::read);
    netCDF::NcFile dataFileV(fileVPath, netCDF::NcFile::read);
    netCDF::NcFile dataFileW(fileWPath, netCDF::NcFile::read);

    LOGI("NetCDF files opened");

    for (size_t i = 0; i < 1; i++) {
        std::vector<size_t> startp = {i, 1, 0, 0};  // Start index for time, depth, y, x
        std::vector<size_t> countp = {1, dataFileU.getDim("depth").getSize()-1, dataFileU.getDim("lat").getSize(), dataFileU.getDim("lon").getSize()};  // Read one time step, all depths, all y, all x
        std::vector<float> uData( countp[1] * countp[2] * countp[3]), vData(countp[1] * countp[2] * countp[3]), wData(countp[1] * countp[2] * countp[3]);
        // Prepare vertex data for OpenGL from uData and vData, and store in allVertices[i]
        width = countp[3];
        height = countp[2];
        depth = countp[1];

        LOGI("HERE");
        dataFileU.getVar("u").getVar(startp, countp, uData.data());
        dataFileV.getVar("v").getVar(startp, countp, vData.data());
        dataFileW.getVar("w").getVar(startp, countp, wData.data());

        LOGI("Data loaded with width: %d, height: %d, depth: %d", width, height, depth);
        prepareVertexData(uData, vData, wData);
    }
}

void VectorFieldHandler::draw(GLShaderManager& shaderManager) {
    // y = [0] + t / T * ([0]-[1])
    std::vector<float> vertices(displayVertices[0].size());
    for (int i = 0; i < displayVertices[0].size(); i++) {
        vertices[i] = displayVertices[0][i] + global_time_in_step / (float) TIME_STEP_IN_SECONDS * (displayVertices[1][i] - displayVertices[0][i]);
    }
    shaderManager.loadVectorFieldData(vertices);
    shaderManager.drawVectorField(vertices.size());
}