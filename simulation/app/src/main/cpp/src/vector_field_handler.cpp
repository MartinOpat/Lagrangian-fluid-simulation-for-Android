//
// Created by martin on 08-05-2024.
//

#include "include/vector_field_handler.h"

VectorFieldHandler::VectorFieldHandler(int finenessXY, int finenessZ): finenessXY(finenessXY), finenessZ(finenessZ) {}

void VectorFieldHandler::velocityField(const glm::vec3 &position, glm::vec3 &velocity) {
    // Transform position [-1, 1] range to [0, adjWidth/adjHeight] grid indices as floating point
    float fGridX = ((position.x / (float)FIELD_WIDTH + 1.0) / 2 * width);
    float fGridY = ((position.y / (float)FIELD_HEIGHT + 1.0) / 2 * height);
    float fGridZ = ((position.z / (float)FIELD_DEPTH + 1.0) / 2 * depth);

    // Calculate base indices by casting to int
    int baseGridX = (int)fGridX;
    int baseGridY = (int)fGridY;
    int baseGridZ = (int)fGridZ;

    // Ensure base indices are within bounds
    baseGridX = std::max(0, std::min(baseGridX, width - 2));
    baseGridY = std::max(0, std::min(baseGridY, height - 2));
    baseGridZ = std::max(0, std::min(baseGridZ, depth - 2));

    // Compute interpolation weights
    float w_x = fGridX - baseGridX;
    float w_y = fGridY - baseGridY;
    float w_z = fGridZ - baseGridZ;

    // Helper function to calculate velocity vector at a given index
    auto getVelocity = [&](int x, int y, int z, int timeIndex) {
        int idx = z * width * height + y * width + x;
        float dx = allVertices[timeIndex][idx * 6 + 3] - allVertices[timeIndex][idx * 6];
        float dy = allVertices[timeIndex][idx * 6 + 4] - allVertices[timeIndex][idx * 6 + 1];
        float dz = allVertices[timeIndex][idx * 6 + 5] - allVertices[timeIndex][idx * 6 + 2];
        return glm::vec3(dx, dy, dz);
    };

    // Interpolate for each time index and then across time
    glm::vec3 interpolatedVelocity[2];
    for (int t = 0; t < 2; t++) {
        glm::vec3 c000 = getVelocity(baseGridX,     baseGridY,     baseGridZ, t);
        glm::vec3 c100 = getVelocity(baseGridX + 1, baseGridY,     baseGridZ, t);
        glm::vec3 c010 = getVelocity(baseGridX,     baseGridY + 1, baseGridZ, t);
        glm::vec3 c110 = getVelocity(baseGridX + 1, baseGridY + 1, baseGridZ, t);
        glm::vec3 c001 = getVelocity(baseGridX,     baseGridY,     baseGridZ + 1, t);
        glm::vec3 c101 = getVelocity(baseGridX + 1, baseGridY,     baseGridZ + 1, t);
        glm::vec3 c011 = getVelocity(baseGridX,     baseGridY + 1, baseGridZ + 1, t);
        glm::vec3 c111 = getVelocity(baseGridX + 1, baseGridY + 1, baseGridZ + 1, t);

        glm::vec3 c00 = glm::mix(c000, c100, w_x);
        glm::vec3 c01 = glm::mix(c001, c101, w_x);
        glm::vec3 c10 = glm::mix(c010, c110, w_x);
        glm::vec3 c11 = glm::mix(c011, c111, w_x);

        glm::vec3 c0 = glm::mix(c00, c10, w_y);
        glm::vec3 c1 = glm::mix(c01, c11, w_y);

        interpolatedVelocity[t] = glm::mix(c0, c1, w_z);
    }

    velocity = glm::mix(interpolatedVelocity[0], interpolatedVelocity[1], global_time_in_step / (float)TIME_STEP);
}


void VectorFieldHandler::prepareVertexData(const std::vector<float>& uData, const std::vector<float>& vData, const std::vector<float>& wData) {
    std::vector<float> vertices;
    vertices.reserve(width * height * depth * 6);
    std::vector<float> tempDisplayVertices;
    tempDisplayVertices.reserve(getWidth() * getHeight() * getDepth() * 6);

    const float maxU = *std::max_element(uData.begin(), uData.end());
    const float minU = *std::min_element(uData.begin(), uData.end());
    const float maxV = *std::max_element(vData.begin(), vData.end());
    const float minV = *std::min_element(vData.begin(), vData.end());
    const float maxW = *std::max_element(wData.begin(), wData.end());
    const float minW = *std::min_element(wData.begin(), wData.end());
    const float max = std::max({maxU, maxV, maxW});
    const float min = std::min({minU, minV, minW});

    for (int z = 0; z < depth; z++) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {

                int index = z * width * height + y * width + x;

                float normalizedX = FIELD_WIDTH*((x / (float)(width)) * 2 - 1);
                float normalizedY = FIELD_HEIGHT*((y / (float)(height)) * 2 - 1);
                float normalizedZ = FIELD_DEPTH*((z / (float)(depth)) * 2 - 1);


                float normalizedU = 2 * ((uData[index] - min) / (max - min)) - 1;
                float normalizedV = 2 * ((vData[index] - min) / (max - min)) - 1;
                float normalizedW = 2 * ((wData[index] - min) / (max - min)) - 1;

                // Start point
                vertices.push_back(normalizedX);
                vertices.push_back(normalizedY);
                vertices.push_back(normalizedZ);

                // End point
                vertices.push_back(normalizedX + normalizedU);
                vertices.push_back(normalizedY + normalizedV);
                vertices.push_back(normalizedZ + normalizedW);

                // Display vertices are reduced
                if (z % finenessZ != 0 || y % finenessXY != 0 || x % finenessXY != 0) continue;
                tempDisplayVertices.push_back(normalizedX);
                tempDisplayVertices.push_back(normalizedY);
                tempDisplayVertices.push_back(normalizedZ);

                float scaleFactor = 10.0f;
                tempDisplayVertices.push_back(normalizedX + normalizedU*scaleFactor);
                tempDisplayVertices.push_back(normalizedY + normalizedV*scaleFactor);
                tempDisplayVertices.push_back(normalizedZ + normalizedW*scaleFactor);
            }
        }
    }

    // Put the newly created vertices in the correct place
    if (allVertices.size() == 3) {
        allVertices[2] = vertices;
        displayVertices[2] = tempDisplayVertices;
    } else {
        LOGI("vector_field_handler", "Vertices not yet filled, pushing");
        allVertices.push_back(vertices);
        displayVertices.push_back(tempDisplayVertices);
    }
}

void VectorFieldHandler::updateTimeStep() {
    if (allVertices.size() > 2) {
        std::swap(allVertices[0], allVertices[1]);
        std::swap(displayVertices[0], displayVertices[1]);
        std::swap(allVertices[1], allVertices[2]);
        std::swap(displayVertices[1], displayVertices[2]);
    } else if (allVertices.size() > 1) {
        std::swap(allVertices[0], allVertices[1]);
        std::swap(displayVertices[0], displayVertices[1]);

    }
}

void VectorFieldHandler::loadTimeStepHelper(const std::string& fileUPath, const std::string& fileVPath, const std::string& fileWPath) {
    netCDF::NcFile dataFileU(fileUPath, netCDF::NcFile::read);
    netCDF::NcFile dataFileV(fileVPath, netCDF::NcFile::read);
    netCDF::NcFile dataFileW(fileWPath, netCDF::NcFile::read);

    // Define the start and count vectors for the data in the file
    std::vector<size_t> startp = {0, 0, 0, 0};  // Start index for time, depth, y, x
    std::vector<size_t> countp = {1, dataFileU.getDim("depth").getSize(), dataFileU.getDim("lat").getSize(), dataFileU.getDim("lon").getSize()};  // Read one time step, all depths, all y, all x
    std::vector<float> uData( countp[1] * countp[2] * countp[3]), vData(countp[1] * countp[2] * countp[3]), wData(countp[1] * countp[2] * countp[3]);

    // Prepare vertex data for OpenGL from uData and vData, and store in allVertices[i]
    width = countp[3];
    height = countp[2];
    depth = countp[1];

    // Read the data
    dataFileU.getVar("u").getVar(startp, countp, uData.data());
    dataFileV.getVar("v").getVar(startp, countp, vData.data());
    dataFileW.getVar("w").getVar(startp, countp, wData.data());

    prepareVertexData(uData, vData, wData);

    // close the files
    dataFileU.close();
    dataFileV.close();
    dataFileW.close();
    std::remove(fileUPath.c_str());
    std::remove(fileVPath.c_str());
    std::remove(fileWPath.c_str());
}

void VectorFieldHandler::loadTimeStep(int fdU, int fdV, int fdW) {
    NetCDFReader reader;
    std::string tempFileU = reader.writeTempFileFromFD(fdU, "tempU.nc");
    std::string tempFileV = reader.writeTempFileFromFD(fdV, "tempV.nc");
    std::string tempFileW = reader.writeTempFileFromFD(fdW, "tempW.nc");

    if (tempFileU.empty() || tempFileV.empty() || tempFileW.empty()) {
        LOGE("native-lib", "Failed to create temporary files.");
        return;
    }
    loadTimeStepHelper(tempFileU, tempFileV, tempFileW);
}

void VectorFieldHandler::draw(Mainview& mainview) {
    // Interpolate between the two time steps
    // y = [0] + t / T * ([0]-[1])
    std::vector<float> vertices(displayVertices[0].size());
    for (int i = 0; i < displayVertices[0].size(); i++) {
        vertices[i] = displayVertices[0][i] + global_time_in_step / (float) TIME_STEP * (displayVertices[1][i] - displayVertices[0][i]);
    }

    // Load the data into the shader and draw
    mainview.loadVectorFieldData(vertices);
    mainview.drawVectorField(vertices.size());
}