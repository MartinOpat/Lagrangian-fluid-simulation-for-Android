//
// Created by martin on 08-05-2024.
//


#include "include/mainview.h"

Mainview::Mainview(AAssetManager* assetManager) {
    transforms = new Transforms();
    shaderManager = new ShaderManager(assetManager);
    navigCube = new NavigCube();
}

Mainview::~Mainview() {
    glDeleteBuffers(1, &particleVBO);
    glDeleteBuffers(1, &vectorFieldVBO);
    glDeleteBuffers(1, &computeVectorField0SSBO);
    glDeleteBuffers(1, &computeVectorField1SSBO);

    glDeleteVertexArrays(1, &particleVAO);
    glDeleteVertexArrays(1, &vectorFieldVAO);

    delete transforms;
    delete shaderManager;
    delete navigCube;
}



void Mainview::setFrame() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//    glEnable(GL_BLEND); // Enable blending
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//    // Enable face culling
//    glEnable(GL_CULL_FACE);
//    // Cull back faces
//    glCullFace(GL_BACK);
//    // Set front faces as counter-clockwise
//    glFrontFace(GL_CCW);

// Enable depth testing
    glEnable(GL_DEPTH_TEST);
// Set the depth function
    glDepthFunc(GL_LESS);

}


void Mainview::loadUniforms() {
    this->pointSize = glGetUniformLocation(shaderManager->shaderPointsProgram, "uPointSize");
    this->modelLocationPoints = glGetUniformLocation(shaderManager->shaderPointsProgram, "modelTransform");
    this->viewLocationPoints = glGetUniformLocation(shaderManager->shaderPointsProgram, "viewTransform");
    this->projectionLocationPoints = glGetUniformLocation(shaderManager->shaderPointsProgram, "projectionTransform");

    this->modelLocationLines = glGetUniformLocation(shaderManager->shaderLinesProgram, "modelTransform");
    this->projectionLocationLines = glGetUniformLocation(shaderManager->shaderLinesProgram, "projectionTransform");
    this->viewLocationLines = glGetUniformLocation(shaderManager->shaderLinesProgram, "viewTransform");

    this->globalTimeInStepLocation = glGetUniformLocation(shaderManager->shaderComputeProgram, "global_time_in_step");
}



void Mainview::setupGraphics() {
    shaderManager->createShaderPrograms();
    loadUniforms();
    navigCube->loadConstUniforms(shaderManager->shaderUIProgram);
}

void Mainview::createParticlesBuffer(std::vector<float>& particlesPos) {
    glUseProgram(shaderManager->shaderPointsProgram);

    // Create VBO
    glGenBuffers(1, &particleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferData(GL_ARRAY_BUFFER, particlesPos.size() * sizeof(float), particlesPos.data(), GL_STREAM_DRAW);

    // Create VAO
    glGenVertexArrays(1, &particleVAO);
    glBindVertexArray(particleVAO);

    // Enable vertex attribute array
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Unbind VAO and VBO
    glBindVertexArray(0);  // Unbind VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);  // Unbind VBO
}

void Mainview::loadParticlesData(std::vector<float>& particlesPos) {
    glUseProgram(shaderManager->shaderPointsProgram);

    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferData(GL_ARRAY_BUFFER, particlesPos.size() * sizeof(float), particlesPos.data(), GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mainview::drawParticles(int size) {
    glUseProgram(shaderManager->shaderPointsProgram);

    // Load VAO
    glBindVertexArray(particleVAO);

    // Load uniforms
    glUniform1f(pointSize, 15.0f);
    glUniformMatrix4fv(modelLocationPoints, 1, GL_TRUE, &(transforms->modelTransform)[0][0]);
    glUniformMatrix4fv(projectionLocationPoints, 1, GL_TRUE, &(transforms->projectionTransform)[0][0]);
    glUniformMatrix4fv(viewLocationPoints, 1, GL_TRUE, &(transforms->viewTransform)[0][0]);

    // Draw
    glDrawArrays(GL_POINTS, 0, size / 3);

    // Unbind
    glBindVertexArray(0);
}

void Mainview::setupColorMap() {
    // Create and bind the color map texture
    glGenTextures(1, &colorMapTexture);
    glBindTexture(GL_TEXTURE_2D, colorMapTexture);

    GLubyte colorData[256 * 3]; // Simple gradient
    for (int i = 0; i < 256; ++i) {
        colorData[i * 3 + 0] = (GLubyte)i;        // Red gradient
        colorData[i * 3 + 1] = (GLubyte)(255 - i); // Green gradient
        colorData[i * 3 + 2] = 128;               // Constant blue
    }

    // OpenGL ES does not support GL_TEXTURE_1D, so we use a 2D texture with height = 1
    glTexImage2D(GL_TEXTURE, 0, GL_RGB, 256, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, colorData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}


void Mainview::createVectorFieldBuffer(std::vector<float>& vertices) {
    setupColorMap();

    glUseProgram(shaderManager->shaderLinesProgram);

    glGenBuffers(1, &vectorFieldVBO);
    glGenVertexArrays(1, &vectorFieldVAO);

    glBindVertexArray(vectorFieldVAO);
    glBindBuffer(GL_ARRAY_BUFFER, vectorFieldVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STREAM_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    // Interpolation values attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Mainview::loadVectorFieldData(std::vector<float>& verticesOld, std::vector<float>& verticesNew) {
    glUseProgram(shaderManager->shaderLinesProgram);


    // Fill each box side vector but in constant time because we exactly know where the sides values are

    std::vector<float> posXSideOld(grid_depth * grid_height * 6);
    std::vector<float> negXSideOld(grid_depth * grid_height * 6);
    std::vector<float> posYSideOld(grid_width * grid_depth * 6);
    std::vector<float> negYSideOld(grid_width * grid_depth * 6);
    std::vector<float> posZSideOld(grid_width * grid_height * 6);
    std::vector<float> negZSideOld(grid_width * grid_height * 6);

    // Copy +/- z sides
    std::copy_n(verticesOld.begin(), grid_width * grid_height * 6, negZSideOld.begin());
    std::copy_n(verticesOld.end() - grid_width * grid_height * 6, grid_width * grid_height * 6, posZSideOld.begin());
    // Copy +/- y sides
    for (int z = 0; z < grid_depth; z++) {
        int negFieldStartIdx = z * grid_height * grid_width * 6;
        int posFieldStartIdx = negFieldStartIdx + grid_height * grid_width * 6 - grid_width * 6;
        int sideIdx = z * grid_width * 6;
        std::copy_n(&(verticesOld[posFieldStartIdx]), 6 * grid_width, &(posYSideOld[sideIdx]));
        std::copy_n(&(verticesOld[negFieldStartIdx]), 6 * grid_width, &(negYSideOld[sideIdx]));
        // Copy +/- x sides
        for (int y = 0; y < grid_height; y++) {
            int negXFieldStartIdx = y * grid_width * 6 + z * grid_height * grid_width * 6;
            int posXFieldStartIdx = negXFieldStartIdx + grid_width * 6;
            int sideXIdx = z * grid_height * 6 + y*6;
            std::copy_n(&(verticesOld[posXFieldStartIdx]), 6, &(posXSideOld[sideXIdx]));
            std::copy_n(&(verticesOld[negXFieldStartIdx]), 6, &(negXSideOld[sideXIdx]));
        }
    }


    std::vector<float> posXSideNew(grid_depth * grid_height * 6);
    std::vector<float> negXSideNew(grid_depth * grid_height * 6);
    std::vector<float> posYSideNew(grid_width * grid_depth * 6);
    std::vector<float> negYSideNew(grid_width * grid_depth * 6);
    std::vector<float> posZSideNew(grid_width * grid_height * 6);
    std::vector<float> negZSideNew(grid_width * grid_height * 6);

    // Copy +/- z sides
    std::copy_n(verticesNew.begin(), grid_width * grid_height * 6, negZSideNew.begin());
    std::copy_n(verticesNew.end() - grid_width * grid_height * 6, grid_width * grid_height * 6, posZSideNew.begin());
    // Copy +/- y sides
    for (int z = 0; z < grid_depth; z++) {
        int negFieldStartIdx = z * grid_height * grid_width * 6;
        int posFieldStartIdx = negFieldStartIdx + grid_height * grid_width * 6 - grid_width * 6;
        int sideIdx = z * grid_width * 6;
        std::copy_n(&(verticesNew[posFieldStartIdx]), 6 * grid_width, &(posYSideNew[sideIdx]));
        std::copy_n(&(verticesNew[negFieldStartIdx]), 6 * grid_width, &(negYSideNew[sideIdx]));
        // Copy +/- x sides
        for (int y = 0; y < grid_height; y++) {
            int negXFieldStartIdx = y * grid_width * 6 + z * grid_height * grid_width * 6;
            int posXFieldStartIdx = negXFieldStartIdx + grid_width * 6;
            int sideXIdx = z * grid_height * 6 + y*6;
            std::copy_n(&(verticesNew[posXFieldStartIdx]), 6, &(posXSideNew[sideXIdx]));
            std::copy_n(&(verticesNew[negXFieldStartIdx]), 6, &(negXSideNew[sideXIdx]));
        }
    }

    // TODO: Make it less dense here.


    std::vector<float> posXSide(posXSideOld.size());
    std::vector<float> negXSide(negXSideOld.size());
    std::vector<float> posYSide(posYSideOld.size());
    std::vector<float> negYSide(negYSideOld.size());
    std::vector<float> posZSide(posZSideOld.size());
    std::vector<float> negZSide(negZSideOld.size());

    // Interpolate between new and old
    for (int i = 0; i < posXSideOld.size(); i++) {
        posXSide[i] = posXSideOld[i] + global_time_in_step / (float) TIME_STEP * (posXSideNew[i] - posXSideOld[i]);
        negXSide[i] = negXSideOld[i] + global_time_in_step / (float) TIME_STEP * (negXSideNew[i] - negXSideOld[i]);
    }
    for (int i = 0; i < posYSideOld.size(); i++) {
        posYSide[i] = posYSideOld[i] + global_time_in_step / (float) TIME_STEP * (posYSideNew[i] - posYSideOld[i]);
        negYSide[i] = negYSideOld[i] + global_time_in_step / (float) TIME_STEP * (negYSideNew[i] - negYSideOld[i]);
    }
    for (int i = 0; i < posZSideOld.size(); i++) {
        posZSide[i] = posZSideOld[i] + global_time_in_step / (float) TIME_STEP * (posZSideNew[i] - posZSideOld[i]);
        negZSide[i] = negZSideOld[i] + global_time_in_step / (float) TIME_STEP * (negZSideNew[i] - negZSideOld[i]);
    }

    faceTriangles.clear();

    // z+ face
    for (int x = 0; x < grid_width-1; x++) {
        for (int y = 0; y < grid_height-1; y++) {
            faceTriangles.insert(faceTriangles.end(), posZSide.begin() + (x + y * grid_width) * 6, posZSide.begin() + (x + y * grid_width) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), posZSide.begin() + (x + (y + 1) * grid_width) * 6, posZSide.begin() + (x + (y + 1) * grid_width) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), posZSide.begin() + ((x + 1) + y * grid_width) * 6, posZSide.begin() + ((x + 1) + y * grid_width) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), posZSide.begin() + ((x + 1) + y * grid_width) * 6, posZSide.begin() + ((x + 1) + y * grid_width) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), posZSide.begin() + (x + (y + 1) * grid_width) * 6, posZSide.begin() + (x + (y + 1) * grid_width) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), posZSide.begin() + ((x + 1) + (y + 1) * grid_width) * 6, posZSide.begin() + ((x + 1) + (y + 1) * grid_width) * 6 + 6);
        }
    }

    // y+ face
    for (int x = 0; x < grid_width-1; x++) {
        for (int z = 0; z < grid_depth-1; z++) {
            faceTriangles.insert(faceTriangles.end(), posYSide.begin() + (x + z * grid_width) * 6, posYSide.begin() + (x + z * grid_width) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), posYSide.begin() + (x + (z + 1) * grid_width) * 6, posYSide.begin() + (x + (z + 1) * grid_width) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), posYSide.begin() + ((x + 1) + z * grid_width) * 6, posYSide.begin() + ((x + 1) + z * grid_width) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), posYSide.begin() + ((x + 1) + z * grid_width) * 6, posYSide.begin() + ((x + 1) + z * grid_width) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), posYSide.begin() + (x + (z + 1) * grid_width) * 6, posYSide.begin() + (x + (z + 1) * grid_width) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), posYSide.begin() + ((x + 1) + (z + 1) * grid_width) * 6, posYSide.begin() + ((x + 1) + (z + 1) * grid_width) * 6 + 6);
        }
    }

//    // x+ face
    for (int y = 0; y < grid_height-1; y++) {
        for (int z = 0; z < grid_depth-1; z++) {
            faceTriangles.insert(faceTriangles.end(), posXSide.begin() + (y + z * grid_height) * 6, posXSide.begin() + (y + z * grid_height) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), posXSide.begin() + (y + (z + 1) * grid_height) * 6, posXSide.begin() + (y + (z + 1) * grid_height) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), posXSide.begin() + ((y + 1) + z * grid_height) * 6, posXSide.begin() + ((y + 1) + z * grid_height) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), posXSide.begin() + ((y + 1) + z * grid_height) * 6, posXSide.begin() + ((y + 1) + z * grid_height) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), posXSide.begin() + (y + (z + 1) * grid_height) * 6, posXSide.begin() + (y + (z + 1) * grid_height) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), posXSide.begin() + ((y + 1) + (z + 1) * grid_height) * 6, posXSide.begin() + ((y + 1) + (z + 1) * grid_height) * 6 + 6);
        }
    }
//
    // z- face
    for (int x = 0; x < grid_width-1; x++) {
        for (int y = 0; y < grid_height-1; y++) {
            faceTriangles.insert(faceTriangles.end(), negZSide.begin() + (x + y * grid_width) * 6, negZSide.begin() + (x + y * grid_width) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), negZSide.begin() + (x + (y + 1) * grid_width) * 6, negZSide.begin() + (x + (y + 1) * grid_width) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), negZSide.begin() + ((x + 1) + y * grid_width) * 6, negZSide.begin() + ((x + 1) + y * grid_width) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), negZSide.begin() + ((x + 1) + y * grid_width) * 6, negZSide.begin() + ((x + 1) + y * grid_width) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), negZSide.begin() + (x + (y + 1) * grid_width) * 6, negZSide.begin() + (x + (y + 1) * grid_width) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), negZSide.begin() + ((x + 1) + (y + 1) * grid_width) * 6, negZSide.begin() + ((x + 1) + (y + 1) * grid_width) * 6 + 6);
        }
    }
//
    // y- face
    for (int x = 0; x < grid_width-1; x++) {
        for (int z = 0; z < grid_depth-1; z++) {
            faceTriangles.insert(faceTriangles.end(), negYSide.begin() + (x + z * grid_width) * 6, negYSide.begin() + (x + z * grid_width) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), negYSide.begin() + (x + (z + 1) * grid_width) * 6, negYSide.begin() + (x + (z + 1) * grid_width) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), negYSide.begin() + ((x + 1) + z * grid_width) * 6, negYSide.begin() + ((x + 1) + z * grid_width) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), negYSide.begin() + ((x + 1) + z * grid_width) * 6, negYSide.begin() + ((x + 1) + z * grid_width) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), negYSide.begin() + (x + (z + 1) * grid_width) * 6, negYSide.begin() + (x + (z + 1) * grid_width) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), negYSide.begin() + ((x + 1) + (z + 1) * grid_width) * 6, negYSide.begin() + ((x + 1) + (z + 1) * grid_width) * 6 + 6);
        }
    }
//
//    // x- face
    for (int y = 0; y < grid_height-1; y++) {
        for (int z = 0; z < grid_depth-1; z++) {
            faceTriangles.insert(faceTriangles.end(), negXSide.begin() + (y + z * grid_height) * 6, negXSide.begin() + (y + z * grid_height) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), negXSide.begin() + (y + (z + 1) * grid_height) * 6, negXSide.begin() + (y + (z + 1) * grid_height) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), negXSide.begin() + ((y + 1) + z * grid_height) * 6, negXSide.begin() + ((y + 1) + z * grid_height) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), negXSide.begin() + ((y + 1) + z * grid_height) * 6, negXSide.begin() + ((y + 1) + z * grid_height) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), negXSide.begin() + (y + (z + 1) * grid_height) * 6, negXSide.begin() + (y + (z + 1) * grid_height) * 6 + 6);
            faceTriangles.insert(faceTriangles.end(), negXSide.begin() + ((y + 1) + (z + 1) * grid_height) * 6, negXSide.begin() + ((y + 1) + (z + 1) * grid_height) * 6 + 6);
        }
    }

    glBindVertexArray(vectorFieldVAO);
    glBindBuffer(GL_ARRAY_BUFFER, vectorFieldVBO);
    glBufferData(GL_ARRAY_BUFFER, faceTriangles.size() * sizeof(float), faceTriangles.data(), GL_STREAM_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mainview::drawVectorField(int size) {
    // Load VAOs
    glUseProgram(shaderManager->shaderLinesProgram);

    glBindVertexArray(vectorFieldVAO);
    glUniformMatrix4fv(modelLocationLines, 1, GL_TRUE, &(transforms->modelTransform)[0][0]);
    glUniformMatrix4fv(projectionLocationLines, 1, GL_TRUE, &transforms->projectionTransform[0][0]);
    glUniformMatrix4fv(viewLocationLines, 1, GL_TRUE, &transforms->viewTransform[0][0]);


//    LOGI("Mainview", "Drawing vector field of size: %zu", faceTriangles.size());
    glDrawArrays(GL_TRIANGLES, 0, faceTriangles.size() / 6);

    glBindVertexArray(0);
}

// Create Buffers
void Mainview::createComputeBuffer(std::vector<float>& vector_field0, std::vector<float>& vector_field1, std::vector<float>& vector_field2) {
    // Create SSBO for previous vector field
    glGenBuffers(1, &computeVectorField0SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, computeVectorField0SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, vector_field0.size() * sizeof(float), vector_field0.data(), GL_DYNAMIC_DRAW);

    // Create SSBO for next vector field
    glGenBuffers(1, &computeVectorField1SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, computeVectorField1SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, vector_field1.size() * sizeof(float), vector_field1.data(), GL_DYNAMIC_DRAW);

    // Create SSBO for vector field to be loaded next
    glGenBuffers(1, &computeVectorField2SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, computeVectorField2SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, vector_field2.size() * sizeof(float), vector_field2.data(), GL_DYNAMIC_DRAW);

    // Unbind
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Mainview::loadConstUniforms(float dt, int width, int height, int depth) {
    glUseProgram(shaderManager->shaderComputeProgram);

    glUniform1i(glGetUniformLocation(shaderManager->shaderComputeProgram, "width"), width);
    glUniform1i(glGetUniformLocation(shaderManager->shaderComputeProgram, "height"), height);
    glUniform1i(glGetUniformLocation(shaderManager->shaderComputeProgram, "depth"), depth);
    glUniform1f(glGetUniformLocation(shaderManager->shaderComputeProgram, "TIME_STEP"), (float) TIME_STEP);
    glUniform1f(glGetUniformLocation(shaderManager->shaderComputeProgram, "dt"), dt);
    glUniform1f(glGetUniformLocation(shaderManager->shaderComputeProgram, "max_width"), (float)FIELD_WIDTH);
    glUniform1f(glGetUniformLocation(shaderManager->shaderComputeProgram, "max_height"), (float)FIELD_HEIGHT);
    glUniform1f(glGetUniformLocation(shaderManager->shaderComputeProgram, "max_depth"), (float)FIELD_DEPTH);
}

void Mainview::preloadComputeBuffer(std::vector<float>& vector_field, std::atomic<GLsync>& globalFence) {
    // Load the new vector field into the SSBO not used for graphing
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, computeVectorField2SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, vector_field.size() * sizeof(float), vector_field.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Setup fence to make sure the data is loaded and sync between threads before use
    GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    glFlush();
    globalFence.store(fence, std::memory_order_release);
}

void Mainview::loadComputeBuffer() {
    std::swap(computeVectorField0SSBO, computeVectorField1SSBO);
    std::swap(computeVectorField1SSBO, computeVectorField2SSBO);
}

void Mainview::dispatchComputeShader() {
    glUseProgram(shaderManager->shaderComputeProgram);

    // Load uniforms
    glUniform1f(globalTimeInStepLocation, global_time_in_step);

    // Bind SSBOs
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particleVBO); // Bind VBO as SSBO
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, computeVectorField0SSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, computeVectorField1SSBO);

    // Dispatch
    glDispatchCompute((NUM_PARTICLES+255) / 256, 1, 1);

    // Ensure vertex shader sees the updates
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
}

void Mainview::drawUI() {
    glm::vec3 rot = transforms->getRotation();

    glm::mat4x4 modelTransform = glm::identity<glm::mat4>();
    modelTransform *= glm::rotate(glm::identity<glm::mat4>(), rot.x, glm::vec3(1.0f, 0.0f, 0.0f));
    modelTransform *= glm::rotate(glm::identity<glm::mat4>(), -rot.y, glm::vec3(0.0f, 1.0f, 0.0f));
    modelTransform *= glm::rotate(glm::identity<glm::mat4>(), rot.z, glm::vec3(0.0f, 0.0f, 1.0f));

    navigCube->draw(shaderManager->shaderUIProgram, modelTransform);
}
