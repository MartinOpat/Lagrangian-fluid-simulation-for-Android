#include "mainview.h"

Mainview::Mainview(AAssetManager* assetManager)
        : assetManager(assetManager), shaderLinesProgram(0), vertexShader(0), fragmentShader(0), textureID(0) {
    startTime = std::chrono::steady_clock::now();
    modelTransform = glm::identity<glm::mat4>();
    projectionTransform = glm::identity<glm::mat4>();
    float max_dim = std::max(FIELD_WIDTH, std::max(FIELD_HEIGHT, FIELD_DEPTH));
    projectionTransform = glm::ortho(-max_dim, max_dim, -max_dim, max_dim, -max_dim, max_dim);
    viewTransform = glm::identity<glm::mat4>();
    setRotation(0.0f, 0.0f, M_PI/2.0f);
    setScale(0.5f);
    updateTransformations();
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
}

void Mainview::setRotation(float rotateX, float rotateY, float rotateZ) {
    this->rotation = glm::vec3(rotateX, rotateY, rotateZ);
    updateTransformations();
}

void Mainview::setScale(float scale) {
    this->scale = scale;
    updateTransformations();
}

void Mainview::updateTransformations() {
    modelTransform = glm::identity<glm::mat4>();
    modelTransform = glm::scale(modelTransform, glm::vec3(scale, scale, scale));

    // Rotation behaviour is order dependent
    modelTransform *= glm::rotate(glm::identity<glm::mat4>(), rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    modelTransform *= glm::rotate(glm::identity<glm::mat4>(), -rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    modelTransform *= glm::rotate(glm::identity<glm::mat4>(), rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
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

void Mainview::compileAndLinkShaders() {
    GLint compileSuccess = 0;
    GLint linkSuccess = 0;
    GLchar infoLog[512];

    // Compile vertex shader
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexShaderSourceCStr = vertexShaderSource.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderSourceCStr, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileSuccess);
    if (!compileSuccess) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        LOGE("mainview", "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s", infoLog);
    }

    // Compile fragment shader
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentShaderSourceCStr = fragmentShaderSource.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderSourceCStr, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileSuccess);
    if (!compileSuccess) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        LOGE("mainview", "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s", infoLog);
    }

    // Compile lines geometry shader
    geometryLinesShader = glCreateShader(GL_GEOMETRY_SHADER);
    const char* geometryShaderSourceCStr = geometryLinesShaderSource.c_str();
    glShaderSource(geometryLinesShader, 1, &geometryShaderSourceCStr, NULL);
    glCompileShader(geometryLinesShader);
    glGetShaderiv(geometryLinesShader, GL_COMPILE_STATUS, &compileSuccess);
    if (!compileSuccess) {
        glGetShaderInfoLog(geometryLinesShader, 512, NULL, infoLog);
        LOGE("mainview", "ERROR::SHADER::GEOMETRY::LINE::COMPILATION_FAILED\n%s", infoLog);
    }

    // Compile points geometry shader
    geometryPointsShader = glCreateShader(GL_GEOMETRY_SHADER);
    const char* geometryPointsShaderSourceCStr = geometryPointsShaderSource.c_str();
    glShaderSource(geometryPointsShader, 1, &geometryPointsShaderSourceCStr, NULL);
    glCompileShader(geometryPointsShader);
    glGetShaderiv(geometryPointsShader, GL_COMPILE_STATUS, &compileSuccess);
    if (!compileSuccess) {
        glGetShaderInfoLog(geometryPointsShader, 512, NULL, infoLog);
        LOGE("mainview", "ERROR::SHADER::GEOMETRY::POINT::COMPILATION_FAILED\n%s", infoLog);
    }

    // Link shaders
    shaderLinesProgram = glCreateProgram();
    glAttachShader(shaderLinesProgram, vertexShader);
    glAttachShader(shaderLinesProgram, geometryLinesShader);
    glAttachShader(shaderLinesProgram, fragmentShader);
    glBindAttribLocation(shaderLinesProgram, 0, "vPosition");
    glLinkProgram(shaderLinesProgram);
    glGetProgramiv(shaderLinesProgram, GL_LINK_STATUS, &linkSuccess);
    if (!linkSuccess) {
        glGetProgramInfoLog(shaderLinesProgram, 512, NULL, infoLog);
        LOGE("mainview", "ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s", infoLog);

    }
    glDetachShader(shaderLinesProgram, vertexShader);
    glDetachShader(shaderLinesProgram, geometryLinesShader);
    glDetachShader(shaderLinesProgram, fragmentShader);

    shaderPointsProgram = glCreateProgram();
    glAttachShader(shaderPointsProgram, vertexShader);
    glAttachShader(shaderPointsProgram, geometryPointsShader);
    glAttachShader(shaderPointsProgram, fragmentShader);
    glBindAttribLocation(shaderPointsProgram, 0, "vPosition");
    glLinkProgram(shaderPointsProgram);
    glGetProgramiv(shaderPointsProgram, GL_LINK_STATUS, &linkSuccess);
    if (!linkSuccess) {
        glGetProgramInfoLog(shaderPointsProgram, 512, NULL, infoLog);
        LOGE("mainview", "ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s", infoLog);
    }

    glDetachShader(shaderPointsProgram, vertexShader);
    glDetachShader(shaderPointsProgram, geometryPointsShader);
    glDetachShader(shaderPointsProgram, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(geometryPointsShader);
    glDeleteShader(geometryLinesShader);
    glDeleteShader(fragmentShader);

    // Compute shader
    computeShader = glCreateShader(GL_COMPUTE_SHADER);
    const char* computeShaderSourceCStr = computeShaderSource.c_str();
    glShaderSource(computeShader, 1, &computeShaderSourceCStr, NULL);
    glCompileShader(computeShader);
    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &compileSuccess);
    if (!compileSuccess) {
        glGetShaderInfoLog(computeShader, 512, NULL, infoLog);
        LOGE("mainview", "ERROR::SHADER::COMPUTE::COMPILATION_FAILED\n%s", infoLog);
    }
    shaderComputeProgram = glCreateProgram();
    glAttachShader(shaderComputeProgram, computeShader);
    glLinkProgram(shaderComputeProgram);
    glGetProgramiv(shaderComputeProgram, GL_LINK_STATUS, &linkSuccess);
    if (!linkSuccess) {
        glGetProgramInfoLog(shaderComputeProgram, 512, NULL, infoLog);
        LOGE("mainview", "ERROR::SHADER::COMPUTE::PROGRAM::LINKING_FAILED\n%s", infoLog);
    }
    glDetachShader(shaderComputeProgram, computeShader);
    glDeleteShader(computeShader);
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
    glUniformMatrix4fv(modelLocationPoints, 1, GL_TRUE, &modelTransform[0][0]);
    glUniformMatrix4fv(projectionLocationPoints, 1, GL_TRUE, &projectionTransform[0][0]);
    glUniformMatrix4fv(viewLocationPoints, 1, GL_TRUE, &viewTransform[0][0]);

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
    glUniformMatrix4fv(modelLocationLines, 1, GL_TRUE, &modelTransform[0][0]);
    glUniformMatrix4fv(projectionLocationLines, 1, GL_TRUE, &projectionTransform[0][0]);
    glUniformMatrix4fv(viewLocationLines, 1, GL_TRUE, &viewTransform[0][0]);

    glDrawArrays(GL_LINES, 0, size / 3);

    glBindVertexArray(0);
}

// Create Buffers
void Mainview::createComputeBuffer(std::vector<float>& vector_field_vertices) {
    glGenBuffers(1, &computeVectorField0SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, computeVectorField0SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, vector_field_vertices.size() * sizeof(float), vector_field_vertices.data(), GL_DYNAMIC_DRAW);

    glGenBuffers(1, &computeVectorField1SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, computeVectorField1SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, vector_field_vertices.size() * sizeof(float), vector_field_vertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Mainview::loadComputeBuffer(std::vector<float>& vector_field0, std::vector<float>& vector_field1) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, computeVectorField0SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, vector_field0.size() * sizeof(float), vector_field0.data(), GL_DYNAMIC_DRAW);
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