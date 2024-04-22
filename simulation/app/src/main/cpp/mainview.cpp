#include "mainview.h"

GLShaderManager::GLShaderManager(AAssetManager* assetManager)
        : assetManager(assetManager), shaderProgram(0), vertexShader(0), fragmentShader(0), textureID(0) {
    startTime = std::chrono::steady_clock::now();
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
    // Compile vertex shader
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexShaderSourceCStr = vertexShaderSource.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderSourceCStr, NULL);
    glCompileShader(vertexShader);

    // Compile fragment shader
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentShaderSourceCStr = fragmentShaderSource.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderSourceCStr, NULL);
    glCompileShader(fragmentShader);

    // Link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

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

void GLShaderManager::drawTriangle() {
    glUniform1i(isPointLocation, 0);
    glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void GLShaderManager::drawParticle() {
    glUniform1i(isPointLocation, 1);
    glBindTexture(GL_TEXTURE_2D, textureID); // Bind the texture for the particle
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_POINTS, 0, 1);
}

void GLShaderManager::setupGraphics() {
    vertexShaderSource = loadShaderFile("vertex_shader.glsl");
    fragmentShaderSource = loadShaderFile("fragment_shader.glsl");

    ImageData texData = loadSimpleTGA(this->assetManager, "textures/pt_tex_debug.tga");

    compileAndLinkShaders();

    createTexture(texData);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Triangle vertices
//    static const GLfloat vertices[] = {
//            0.0f,  0.0f, 0.0f,
//            -0.5f, -0.5f, 0.0f,
//            0.5f, -0.5f, 0.0f
//    };
//    glGenBuffers(1, &triangleVBO);
//    glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
//
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
//    glEnableVertexAttribArray(0);
//
//
    static const GLfloat particleVertex[] = {0.0f, -0.25f, 0.0f};
    glGenBuffers(1, &particleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particleVertex), particleVertex, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    this->isPointLocation = glGetUniformLocation(shaderProgram, "uIsPoint");
}

void GLShaderManager::createVectorFieldBuffer(std::vector<float> vertices) {
    glGenBuffers(1, &vectorFieldVBO);
    glBindBuffer(GL_ARRAY_BUFFER, vectorFieldVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
}

void GLShaderManager::loadVectorFieldData(std::vector<float> vertices) {
    glUniform1i(isPointLocation, 0);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, vectorFieldVBO);
}

void GLShaderManager::drawVectorField(int size) {
    // Set the color
//    GLint colorLocation = glGetUniformLocation(shaderProgram, "u_Color");
//    glUniform4f(colorLocation, 1.0f, 0.3f, 1.0f, 1.0f);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);


    glDrawArrays(GL_LINES, 0, size / 3);
    glDisableVertexAttribArray(0);
}