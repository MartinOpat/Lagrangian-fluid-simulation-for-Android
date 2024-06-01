#include "mainview.h"

Mainview::Mainview(AAssetManager* assetManager) {
    startTime = std::chrono::steady_clock::now();
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
    glGenBuffers(1, &particleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferData(GL_ARRAY_BUFFER, particlesPos.size() * sizeof(float), particlesPos.data(), GL_STREAM_DRAW);

    glGenVertexArrays(1, &particleVAO);
    glBindVertexArray(particleVAO);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

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
    glUseProgram(shaderManager->shaderPointsProgram);  // TODO: Possibly temporary

    glBindVertexArray(particleVAO);
    glUniform1i(isPointLocationPoints, 1);
    glUniform1f(pointSize, 15.0f);
    glUniformMatrix4fv(modelLocationPoints, 1, GL_TRUE, &(transforms->modelTransform)[0][0]);
    glUniformMatrix4fv(projectionLocationPoints, 1, GL_TRUE, &(transforms->projectionTransform)[0][0]);
    glUniformMatrix4fv(viewLocationPoints, 1, GL_TRUE, &(transforms->viewTransform)[0][0]);

    glDrawArrays(GL_POINTS, 0, size / 3);

    glBindVertexArray(0);
}

void Mainview::createVectorFieldBuffer(std::vector<float>& vertices) {
    glUseProgram(shaderManager->shaderLinesProgram);
    glGenBuffers(1, &vectorFieldVBO);
    glBindBuffer(GL_ARRAY_BUFFER, vectorFieldVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STREAM_DRAW);

    glGenVertexArrays(1, &vectorFieldVAO);
    glBindVertexArray(vectorFieldVAO);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);  // Unbind VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);  // Unbind VBO
}

void Mainview::loadVectorFieldData(std::vector<float>& vertices) {
    glUseProgram(shaderManager->shaderLinesProgram);
    glBindBuffer(GL_ARRAY_BUFFER, vectorFieldVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mainview::drawVectorField(int size) {
    glBindVertexArray(vectorFieldVAO);
    glUniform1i(isPointLocationLines, 0);
    glUniformMatrix4fv(modelLocationLines, 1, GL_TRUE, &(transforms->modelTransform)[0][0]);
    glUniformMatrix4fv(projectionLocationLines, 1, GL_TRUE, &transforms->projectionTransform[0][0]);
    glUniformMatrix4fv(viewLocationLines, 1, GL_TRUE, &transforms->viewTransform[0][0]);

    glDrawArrays(GL_LINES, 0, size / 3);

    glBindVertexArray(0);
}

// Create Buffers
void Mainview::createComputeBuffer(std::vector<float>& vector_field0, std::vector<float>& vector_field1, std::vector<float>& vector_field2) {
    glGenBuffers(1, &computeVectorField0SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, computeVectorField0SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, vector_field0.size() * sizeof(float), vector_field0.data(), GL_DYNAMIC_DRAW);

    glGenBuffers(1, &computeVectorField1SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, computeVectorField1SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, vector_field1.size() * sizeof(float), vector_field1.data(), GL_DYNAMIC_DRAW);

    glGenBuffers(1, &computeVectorField2SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, computeVectorField2SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, vector_field2.size() * sizeof(float), vector_field2.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Mainview::loadConstUniforms(float dt, int width, int height, int depth) {
    glUseProgram(shaderManager->shaderComputeProgram);
    glUniform1i(glGetUniformLocation(shaderManager->shaderComputeProgram, "width"), width);
    glUniform1i(glGetUniformLocation(shaderManager->shaderComputeProgram, "height"), height);
    glUniform1i(glGetUniformLocation(shaderManager->shaderComputeProgram, "depth"), depth);
    glUniform1f(glGetUniformLocation(shaderManager->shaderComputeProgram, "TIME_STEP_IN_SECONDS"), (float) TIME_STEP_IN_SECONDS);
    glUniform1f(glGetUniformLocation(shaderManager->shaderComputeProgram, "dt"), dt);
    glUniform1f(glGetUniformLocation(shaderManager->shaderComputeProgram, "max_width"), (float)FIELD_WIDTH);
    glUniform1f(glGetUniformLocation(shaderManager->shaderComputeProgram, "max_height"), (float)FIELD_HEIGHT);
    glUniform1f(glGetUniformLocation(shaderManager->shaderComputeProgram, "max_depth"), (float)FIELD_DEPTH);
}

void Mainview::preloadComputeBuffer(std::vector<float>& vector_field, std::atomic<GLsync>& globalFence) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, computeVectorField2SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, vector_field.size() * sizeof(float), vector_field.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    glFlush();
    globalFence.store(fence, std::memory_order_release);
}

void Mainview::loadComputeBuffer() {
    std::swap(computeVectorField0SSBO, computeVectorField1SSBO);
    std::swap(computeVectorField1SSBO, computeVectorField2SSBO);
}

// Dispatch Compute Shader
void Mainview::dispatchComputeShader() {
    glUseProgram(shaderManager->shaderComputeProgram);
    glUniform1f(globalTimeInStepLocation, global_time_in_step);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particleVBO); // Bind VBO as SSBO
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, computeVectorField0SSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, computeVectorField1SSBO);

    glDispatchCompute((NUM_PARTICLES+255) / 256, 1, 1);
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT); // Ensure vertex shader sees the updates
}