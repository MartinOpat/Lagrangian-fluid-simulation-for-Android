#include "mainview.h"

Mainview::Mainview(AAssetManager* assetManager)
        : assetManager(assetManager), shaderLinesProgram(0), vertexShader(0), fragmentShader(0) {
    startTime = std::chrono::steady_clock::now();
    transforms = new Transforms();
}

Mainview::~Mainview() {
    glDeleteProgram(shaderLinesProgram);
    glDeleteProgram(shaderPointsProgram);
    glDeleteProgram(shaderComputeProgram);

    glDeleteBuffers(1, &particleVBO);
    glDeleteBuffers(1, &vectorFieldVBO);
    glDeleteBuffers(1, &computeVectorField0SSBO);
    glDeleteBuffers(1, &computeVectorField1SSBO);

    glDeleteVertexArrays(1, &particleVAO);
    glDeleteVertexArrays(1, &vectorFieldVAO);

    delete transforms;
}

std::string Mainview::loadShaderFile(const char* fileName) {
    AAsset* asset = AAssetManager_open(assetManager, fileName, AASSET_MODE_BUFFER);
    if (!asset) return "";

    size_t size = AAsset_getLength(asset);
    std::string buffer(size, ' ');
    AAsset_read(asset, &buffer[0], size);
    AAsset_close(asset);

    return buffer;
}

void compileShaderHelper(GLuint& shader, const std::string& shaderSource, GLenum type) {
    GLint compileSuccess = 0;
    GLchar infoLog[512];
    shader = glCreateShader(type);
    const char* vertexShaderSourceCStr = shaderSource.c_str();
    glShaderSource(shader, 1, &vertexShaderSourceCStr, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileSuccess);
    if (!compileSuccess) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        LOGE("mainview", "ERROR::SHADER::COMPILATION_FAILED\n%s", infoLog);
    }
}

void Mainview::compileVertexShader() {
    compileShaderHelper(vertexShader, vertexShaderSource, GL_VERTEX_SHADER);
}

void Mainview::compileFragmentShader() {
    compileShaderHelper(fragmentShader, fragmentShaderSource, GL_FRAGMENT_SHADER);
}

void Mainview::compileLinesGeometryShader() {
    compileShaderHelper(geometryLinesShader, geometryLinesShaderSource, GL_GEOMETRY_SHADER);
}

void Mainview::compilePointsGeometryShader() {
    compileShaderHelper(geometryPointsShader, geometryPointsShaderSource, GL_GEOMETRY_SHADER);
}

void Mainview::compileComputeShaders() {
    compileShaderHelper(computeShader, computeShaderSource, GL_COMPUTE_SHADER);
}

void createProgramHelper(GLuint& program, GLuint shaders[]) {
    GLint linkSuccess = 0;
    GLchar infoLog[512];
    program = glCreateProgram();

    while (*shaders) {
        glAttachShader(program, *shaders);
        shaders++;
    }

    glBindAttribLocation(program, 0, "vPosition");
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linkSuccess);
    if (!linkSuccess) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        LOGE("mainview", "ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s", infoLog);
    }
}

void Mainview::createLinesProgram() {
    createProgramHelper(shaderLinesProgram, (GLuint[]) {vertexShader, geometryLinesShader, fragmentShader, 0});
}

void Mainview::createPointsProgram() {
    createProgramHelper(shaderPointsProgram, (GLuint[]) {vertexShader, geometryPointsShader, fragmentShader, 0});
}

void Mainview::createShaderProgram() {
    createProgramHelper(shaderComputeProgram, (GLuint[]) {computeShader, 0});
}

void Mainview::detachShaders() {
    glDetachShader(shaderLinesProgram, vertexShader);
    glDetachShader(shaderLinesProgram, geometryLinesShader);
    glDetachShader(shaderLinesProgram, fragmentShader);
    glDetachShader(shaderPointsProgram, vertexShader);
    glDetachShader(shaderPointsProgram, geometryPointsShader);
    glDetachShader(shaderPointsProgram, fragmentShader);
    glDetachShader(shaderComputeProgram, computeShader);
}

void Mainview::deleteShaders() {
    glDeleteShader(vertexShader);
    glDeleteShader(geometryPointsShader);
    glDeleteShader(geometryLinesShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(computeShader);
}


void Mainview::compileAndLinkShaders() {
    compileVertexShader();
    compileFragmentShader();
    compileLinesGeometryShader();
    compilePointsGeometryShader();
    compileComputeShaders();

    createLinesProgram();
    createPointsProgram();
    createShaderProgram();

    detachShaders();
    deleteShaders();
}


void Mainview::setFrame() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND); // Enable blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Mainview::setupGraphics() {


    // Load shader source code
    vertexShaderSource = loadShaderFile("vertex_shader.glsl");
    fragmentShaderSource = loadShaderFile("fragment_shader.glsl");
    geometryLinesShaderSource = loadShaderFile("geometry_lines_shader.glsl");
    geometryPointsShaderSource = loadShaderFile("geometry_points_shader.glsl");
    computeShaderSource = loadShaderFile("compute_shader.glsl");

    // Compile and link shaders
    compileAndLinkShaders();

    // Check shader program link status
    GLint linked;
    glGetProgramiv(shaderLinesProgram, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLchar linkLog[1024];
        glGetProgramInfoLog(shaderLinesProgram, sizeof(linkLog), NULL, linkLog);
        LOGE("mainview", "Shader Program Link Error: %s", linkLog);
    }

    GLint linkedPoints;
    glGetProgramiv(shaderPointsProgram, GL_LINK_STATUS, &linkedPoints);
    if (!linkedPoints) {
        GLchar linkLog[1024];
        glGetProgramInfoLog(shaderPointsProgram, sizeof(linkLog), NULL, linkLog);
        LOGE("mainview", "Shader Program Link Error: %s", linkLog);
    }


    // Query uniform locations
    this->isPointLocationPoints = glGetUniformLocation(shaderPointsProgram, "uIsPoint");
    this->pointSize = glGetUniformLocation(shaderPointsProgram, "uPointSize");
    this->modelLocationPoints = glGetUniformLocation(shaderPointsProgram, "modelTransform");
    this->viewLocationPoints = glGetUniformLocation(shaderPointsProgram, "viewTransform");
    this->projectionLocationPoints = glGetUniformLocation(shaderPointsProgram, "projectionTransform");

    this->isPointLocationLines = glGetUniformLocation(shaderLinesProgram, "uIsPoint");
    this->modelLocationLines = glGetUniformLocation(shaderLinesProgram, "modelTransform");
    this->projectionLocationLines = glGetUniformLocation(shaderLinesProgram, "projectionTransform");
    this->viewLocationLines = glGetUniformLocation(shaderLinesProgram, "viewTransform");

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        LOGE("mainview", "OpenGL setup error: %x", err);
    }

    // Cleanup
    vertexShaderSource.clear();
    fragmentShaderSource.clear();
    geometryLinesShaderSource.clear();
    geometryPointsShaderSource.clear();
    computeShaderSource.clear();
}

void Mainview::createParticlesBuffer(std::vector<float>& particlesPos) {
    glUseProgram(shaderPointsProgram);
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
    glUseProgram(shaderPointsProgram);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferData(GL_ARRAY_BUFFER, particlesPos.size() * sizeof(float), particlesPos.data(), GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mainview::drawParticles(int size) {
    glUseProgram(shaderPointsProgram);  // TODO: Possibly temporary

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
    glUseProgram(shaderLinesProgram);
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
    glUseProgram(shaderLinesProgram);
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

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Mainview::loadComputeBuffer(std::vector<float>& vector_field0, std::vector<float>& vector_field1) {
    std::swap(computeVectorField0SSBO, computeVectorField1SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, computeVectorField1SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, vector_field1.size() * sizeof(float), vector_field1.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

// Dispatch Compute Shader
void Mainview::dispatchComputeShader(float dt, float global_time_in_step, int width, int height, int depth) {
    glUseProgram(shaderComputeProgram);

    glUniform1i(glGetUniformLocation(shaderComputeProgram, "width"), width);
    glUniform1i(glGetUniformLocation(shaderComputeProgram, "height"), height);
    glUniform1i(glGetUniformLocation(shaderComputeProgram, "depth"), depth);
    glUniform1f(glGetUniformLocation(shaderComputeProgram, "global_time_in_step"), global_time_in_step);
    glUniform1f(glGetUniformLocation(shaderComputeProgram, "TIME_STEP_IN_SECONDS"), (float) TIME_STEP_IN_SECONDS);
    glUniform1f(glGetUniformLocation(shaderComputeProgram, "dt"), dt);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particleVBO); // Bind VBO as SSBO
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, computeVectorField0SSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, computeVectorField1SSBO);

    glDispatchCompute((NUM_PARTICLES+127) / 128, 1, 1);
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT); // Ensure vertex shader sees the updates
}