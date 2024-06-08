//
// Created by martin on 08-05-2024.
//


#include "include/mainview.h"

Mainview::Mainview(AAssetManager* assetManager) {
    transforms = new Transforms();
    shaderManager = new ShaderManager(assetManager);
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
}



void Mainview::setFrame() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND); // Enable blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void Mainview::loadUniforms() {
    this->isPointLocationPoints = glGetUniformLocation(shaderManager->shaderPointsProgram, "uIsPoint");
    this->pointSize = glGetUniformLocation(shaderManager->shaderPointsProgram, "uPointSize");
    this->modelLocationPoints = glGetUniformLocation(shaderManager->shaderPointsProgram, "modelTransform");
    this->viewLocationPoints = glGetUniformLocation(shaderManager->shaderPointsProgram, "viewTransform");
    this->projectionLocationPoints = glGetUniformLocation(shaderManager->shaderPointsProgram, "projectionTransform");

    this->isPointLocationLines = glGetUniformLocation(shaderManager->shaderLinesProgram, "uIsPoint");
    this->modelLocationLines = glGetUniformLocation(shaderManager->shaderLinesProgram, "modelTransform");
    this->projectionLocationLines = glGetUniformLocation(shaderManager->shaderLinesProgram, "projectionTransform");
    this->viewLocationLines = glGetUniformLocation(shaderManager->shaderLinesProgram, "viewTransform");

    this->globalTimeInStepLocation = glGetUniformLocation(shaderManager->shaderComputeProgram, "global_time_in_step");
}



void Mainview::setupGraphics() {
    shaderManager->createShaderPrograms();

    loadUniforms();
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
    glUniform1i(isPointLocationPoints, 1);
    glUniform1f(pointSize, 15.0f);
    glUniformMatrix4fv(modelLocationPoints, 1, GL_TRUE, &(transforms->modelTransform)[0][0]);
    glUniformMatrix4fv(projectionLocationPoints, 1, GL_TRUE, &(transforms->projectionTransform)[0][0]);
    glUniformMatrix4fv(viewLocationPoints, 1, GL_TRUE, &(transforms->viewTransform)[0][0]);

    // Draw
    glDrawArrays(GL_POINTS, 0, size / 3);

    // Unbind
    glBindVertexArray(0);
}

void Mainview::createVectorFieldBuffer(std::vector<float>& vertices) {
    glUseProgram(shaderManager->shaderLinesProgram);

    // Enable vertex attribute array
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(3 * sizeof(float)));

    // Also create relevant texture
    setupTexture();

}

void Mainview::loadVectorFieldData(std::vector<float>& vertices) {
    glUseProgram(shaderManager->shaderLinesProgram);

    std::vector<glm::vec3> processedVectors(36 * 18 * 6 * 3);  // For RGB components only
    for (int i = 0; i < (36 * 18 * 6); i++) {
        processedVectors[i] = glm::vec3(transforms->projectionTransform * transforms->viewTransform * transforms->modelTransform * glm::vec4(
                    (vertices[6*i + 3] - vertices[6*i + 0]),
                    (vertices[6*i + 4] - vertices[6*i + 1]),
                    (vertices[6*i + 5] - vertices[6*i + 2]),
                    1.0f
                ));
    }
    glBindTexture(GL_TEXTURE_3D, vectorFieldTexture);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, 36, 18, 6, GL_RGB, GL_FLOAT, processedVectors.data());
    glBindTexture(GL_TEXTURE_3D, 0);

}

void Mainview::drawVectorField(int size) {
    // Load VAO

    glUniformMatrix4fv(modelLocationLines, 1, GL_TRUE, &(transforms->modelTransform)[0][0]);
    glUniformMatrix4fv(projectionLocationLines, 1, GL_TRUE, &(transforms->projectionTransform)[0][0]);
    glUniformMatrix4fv(viewLocationLines, 1, GL_TRUE, &(transforms->viewTransform)[0][0]);


    // Load uniforms
    glUniform1i(isPointLocationLines, 0);

    // Use texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);
    glUniform1i(glGetUniformLocation(shaderManager->shaderLinesProgram, "noiseTex"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, vectorFieldTexture);
    glUniform1i(glGetUniformLocation(shaderManager->shaderLinesProgram, "vectorFieldTex"), 1);

    // Draw
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Unbind
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_3D, 0);
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

    LOGI("Mainview", "Loading uniforms: dt: %f, width: %d, height: %d, depth: %d", dt, width, height, depth);

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

void Mainview::setupTexture() {

    ImageData texData = loadSimpleTGA(shaderManager->assetManager, "textures/noiseTexture.tga");

    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    // Set texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texData.width, texData.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData.data.data());

    glBindTexture(GL_TEXTURE_2D, 0);





    glGenTextures(1, &vectorFieldTexture);
    glBindTexture(GL_TEXTURE_3D, vectorFieldTexture);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

// Assuming your data is normalized between [0, 1]
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB32F, 36, 18, 6, 0, GL_RGB, GL_FLOAT, nullptr);
}
