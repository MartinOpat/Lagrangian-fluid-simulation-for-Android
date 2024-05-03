#include "mainview.h"


Point scale_point(const Point& p, const Vec3& scale) {
    return Point(p.x * scale.x, p.y * scale.y, p.z * scale.z);
}

Point rotate_point(const Point& p, const Vec3& rotation) {
    double radX = rotation.x;
    double radY = rotation.y;
    double radZ = rotation.z;

    // Rotating around X-axis
    Point rowX1(1, 0, 0);
    Point rowX2(0, cos(radX), -sin(radX));
    Point rowX3(0, sin(radX), cos(radX));

    // Rotating around Y-axis
    Point rowY1(cos(radY), 0, sin(radY));
    Point rowY2(0, 1, 0);
    Point rowY3(-sin(radY), 0, cos(radY));

    // Rotating around Z-axis
    Point rowZ1(cos(radZ), -sin(radZ), 0);
    Point rowZ2(sin(radZ), cos(radZ), 0);
    Point rowZ3(0, 0, 1);

    // Apply rotation X
    Point rotatedX = Point(p.dot(rowX1), p.dot(rowX2), p.dot(rowX3));

    // Apply rotation X
    Point rotatedXY = Point(rotatedX.dot(rowY1), rotatedX.dot(rowY2), rotatedX.dot(rowY3));

    // Apply rotation Z
    Point rotatedXYZ = Point(rotatedXY.dot(rowZ1), rotatedXY.dot(rowZ2), rotatedXY.dot(rowZ3));

    return rotatedXYZ;
}

Point translate(const Point& p, const Point& translation) {
    return Point(p.x + translation.x, p.y + translation.y, p.z + translation.z);
}


void GLShaderManager::setRotation(float rotateX, float rotateY, float rotateZ) {
    this->rotation = Vec3(rotateX, rotateY, rotateZ);
    updateTransformations();
}

void GLShaderManager::setScale(float scale) {
    this->scale = scale;
    updateTransformations();
}

void GLShaderManager::updateTransformations() {
    modelTransform.setToIdentity();
    modelTransform = modelTransform * Matrix4x4::scale(scale, scale, scale);
    modelTransform = modelTransform * Matrix4x4::rotateX(rotation.x);
    modelTransform = modelTransform * Matrix4x4::rotateY(rotation.y);
    modelTransform = modelTransform * Matrix4x4::rotateZ(rotation.z);
}



GLShaderManager::GLShaderManager(AAssetManager* assetManager)
        : assetManager(assetManager), shaderProgram(0), vertexShader(0), fragmentShader(0), textureID(0) {
    startTime = std::chrono::steady_clock::now();
//    setRotation(0.78f, 0.78f, 0.78f);
}

GLShaderManager::~GLShaderManager() {
    glDeleteProgram(shaderProgram);
    glDeleteTextures(1, &textureID);
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
//        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
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
//        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        LOGE("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s", infoLog);
    }

    // Link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindAttribLocation(shaderProgram, 0, "vPosition");
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkSuccess);
    if (!linkSuccess) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
//        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        LOGE("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s", infoLog);
    }

    // Cleanup
    glDetachShader(shaderProgram, vertexShader);
    glDetachShader(shaderProgram, fragmentShader);
    glDeleteShader(vertexShader);
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
    glUniformMatrix4fv(modelLocation, 1, GL_TRUE, modelTransform.data);

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

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
    glUniformMatrix4fv(modelLocation, 1, GL_TRUE, modelTransform.data);

    glDrawArrays(GL_LINES, 0, size / 3);

    glBindVertexArray(0);
}