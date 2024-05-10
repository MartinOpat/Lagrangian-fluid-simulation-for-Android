#include "mainview.h"

GLShaderManager::GLShaderManager(AAssetManager* assetManager)
        : assetManager(assetManager), shaderProgram(0), vertexShader(0), fragmentShader(0), textureID(0) {
    startTime = std::chrono::steady_clock::now();
    modelTransform = glm::identity<glm::mat4>();
    projectionTransform = glm::identity<glm::mat4>();
//    projectionTransform = glm::perspective(glm::radians(60.0f), 20.0f/9.0f, 0.2f, 20.0f);
//    projectionTransform = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 10.0f);
    float max_dim = std::max(FIELD_WIDTH, std::max(FIELD_HEIGHT, FIELD_DEPTH));
    projectionTransform = glm::ortho(-max_dim, max_dim, -max_dim, max_dim, -max_dim, max_dim);
    viewTransform = glm::identity<glm::mat4>();
//    viewTransform = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    setRotation(0.0f, 0.0f, M_PI/2.0f);
//    setRotation(0.0f, 0.0f, 0.0f);
    setScale(0.5f);
//    setScale(1.0f);
    updateTransformations();
}

GLShaderManager::~GLShaderManager() {
    glDeleteProgram(shaderProgram);
    glDeleteTextures(1, &textureID);
}

void GLShaderManager::setRotation(float rotateX, float rotateY, float rotateZ) {
    this->rotation = glm::vec3(rotateX, rotateY, rotateZ);
    updateTransformations();
}

void GLShaderManager::setScale(float scale) {
    this->scale = scale;
    updateTransformations();
}

void GLShaderManager::updateTransformations() {
    modelTransform = glm::identity<glm::mat4>();
    modelTransform = glm::scale(modelTransform, glm::vec3(scale, scale, scale));

    // Rotation behaviour is order dependent
    modelTransform *= glm::rotate(glm::identity<glm::mat4>(), rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    modelTransform *= glm::rotate(glm::identity<glm::mat4>(), -rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    modelTransform *= glm::rotate(glm::identity<glm::mat4>(), rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
}




std::string GLShaderManager::loadShaderFile(const char* fileName) {
    AAsset* asset = AAssetManager_open(assetManager, fileName, AASSET_MODE_BUFFER);
    if (!asset) return "";

    size_t size = AAsset_getLength(asset);
    std::string buffer(size, ' ');
    AAsset_read(asset, &buffer[0], size);
    AAsset_close(asset);

    return buffer;
}

void GLShaderManager::createTexture(const ImageData& texData) {
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texData.width, texData.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData.data.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void GLShaderManager::compileAndLinkShaders() {
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
        LOGE("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s", infoLog);
    }

    // Compile fragment shader
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentShaderSourceCStr = fragmentShaderSource.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderSourceCStr, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileSuccess);
    if (!compileSuccess) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        LOGE("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s", infoLog);
    }

    // Compile geometry shader
    geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
    const char* geometryShaderSourceCStr = geometryShaderSource.c_str();
    glShaderSource(geometryShader, 1, &geometryShaderSourceCStr, NULL);
    glCompileShader(geometryShader);
    glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &compileSuccess);
    if (!compileSuccess) {
        glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);
        LOGE("ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n%s", infoLog);
    }

    // Link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, geometryShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindAttribLocation(shaderProgram, 0, "vPosition");
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkSuccess);
    if (!linkSuccess) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        LOGE("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s", infoLog);
    }

    // Cleanup
    glDetachShader(shaderProgram, vertexShader);
    glDetachShader(shaderProgram, geometryShader);
    glDetachShader(shaderProgram, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader);
}


void GLShaderManager::setFrame() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND); // Enable blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(shaderProgram);
}

void GLShaderManager::setupGraphics() {


    // Load shader source code
    vertexShaderSource = loadShaderFile("vertex_shader.glsl");
    fragmentShaderSource = loadShaderFile("fragment_shader.glsl");
    geometryShaderSource = loadShaderFile("geometry_shader.glsl");

    // Compile and link shaders
    compileAndLinkShaders();

    // Check shader program link status
    GLint linked;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLchar linkLog[1024];
        glGetProgramInfoLog(shaderProgram, sizeof(linkLog), NULL, linkLog);
        LOGE("Shader Program Link Error: %s", linkLog);
    }

    // Query uniform locations
    this->isPointLocation = glGetUniformLocation(shaderProgram, "uIsPoint");
    this->pointSize = glGetUniformLocation(shaderProgram, "uPointSize");
    this->modelLocation = glGetUniformLocation(shaderProgram, "modelTransform");
    this->projectionLocation = glGetUniformLocation(shaderProgram, "projectionTransform");
    this->viewLocation = glGetUniformLocation(shaderProgram, "viewTransform");

    // Error checking after setup
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        LOGE("OpenGL setup error: %x", err);
    }
}

void GLShaderManager::createParticlesBuffer(std::vector<float> particlesPos) {
    glGenBuffers(1, &particleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferData(GL_ARRAY_BUFFER, particlesPos.size() * sizeof(float), particlesPos.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &particleVAO);
    glBindVertexArray(particleVAO);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindVertexArray(0);  // Unbind VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);  // Unbind VBO
}

void GLShaderManager::loadParticlesData(std::vector<float> particlesPos) {
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferData(GL_ARRAY_BUFFER, particlesPos.size() * sizeof(float), particlesPos.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLShaderManager::drawParticles(int size) {
    glBindVertexArray(particleVAO);
    glUniform1i(isPointLocation, 1);
    glUniform1f(pointSize, 15.0f);
    glUniformMatrix4fv(modelLocation, 1, GL_TRUE, &modelTransform[0][0]);
    glUniformMatrix4fv(projectionLocation, 1, GL_TRUE, &projectionTransform[0][0]);
    glUniformMatrix4fv(viewLocation, 1, GL_TRUE, &viewTransform[0][0]);

    glDrawArrays(GL_POINTS, 0, size / 3);

    glBindVertexArray(0);
}

void GLShaderManager::createVectorFieldBuffer(std::vector<float> vertices) {
    glGenBuffers(1, &vectorFieldVBO);
    glBindBuffer(GL_ARRAY_BUFFER, vectorFieldVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &vectorFieldVAO);
    glBindVertexArray(vectorFieldVAO);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);  // Unbind VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);  // Unbind VBO
}

void GLShaderManager::loadVectorFieldData(std::vector<float> vertices) {
    glBindBuffer(GL_ARRAY_BUFFER, vectorFieldVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLShaderManager::drawVectorField(int size) {
    glBindVertexArray(vectorFieldVAO);
    glUniform1i(isPointLocation, 0);
    glUniformMatrix4fv(modelLocation, 1, GL_TRUE, &modelTransform[0][0]);
    glUniformMatrix4fv(projectionLocation, 1, GL_TRUE, &projectionTransform[0][0]);
    glUniformMatrix4fv(viewLocation, 1, GL_TRUE, &viewTransform[0][0]);

    glDrawArrays(GL_LINES, 0, size / 3);

    glBindVertexArray(0);
}