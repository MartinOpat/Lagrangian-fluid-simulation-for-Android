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

    // Create VBO
    glGenBuffers(1, &vectorFieldVBO);
    glBindBuffer(GL_ARRAY_BUFFER, vectorFieldVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STREAM_DRAW);

    // Create VAO
    glGenVertexArrays(1, &vectorFieldVAO);
    glBindVertexArray(vectorFieldVAO);

    // Enable vertex attribute array
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // Unbind VAO and VBO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mainview::loadVectorFieldData(std::vector<float>& vertices) {
    glUseProgram(shaderManager->shaderLinesProgram);

    glBindBuffer(GL_ARRAY_BUFFER, vectorFieldVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mainview::drawVectorField(int size) {
    // Load VAO
    glBindVertexArray(vectorFieldVAO);

    // Load uniforms
    glUniform1i(isPointLocationLines, 0);
    glUniformMatrix4fv(modelLocationLines, 1, GL_TRUE, &(transforms->modelTransform)[0][0]);
    glUniformMatrix4fv(projectionLocationLines, 1, GL_TRUE, &transforms->projectionTransform[0][0]);
    glUniformMatrix4fv(viewLocationLines, 1, GL_TRUE, &transforms->viewTransform[0][0]);

    // Draw
    glDrawArrays(GL_LINES, 0, size / 3);

    // Check for errors
    GLenum err;

    // Unbind
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
