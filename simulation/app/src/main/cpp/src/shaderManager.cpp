//
// Created by martin on 01-06-2024.
//

#include "include/shaderManager.h"

ShaderManager::ShaderManager(AAssetManager *assetManager): assetManager(assetManager) {}

ShaderManager::~ShaderManager() {
    glDeleteProgram(shaderLinesProgram);
    glDeleteProgram(shaderPointsProgram);
    glDeleteProgram(shaderComputeProgram);
}


std::string ShaderManager::loadShaderFile(const char* fileName) {
    AAsset* asset = AAssetManager_open(assetManager, fileName, AASSET_MODE_BUFFER);
    if (!asset) return "";

    size_t size = AAsset_getLength(asset);
    std::string buffer(size, ' ');
    AAsset_read(asset, &buffer[0], size);
    AAsset_close(asset);

    return buffer;
}

// Helper function to compile a shader from a `.glsl` file
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
        LOGE("shaderManager", "ERROR::SHADER::COMPILATION_FAILED\n%s", infoLog);
    }
}

void ShaderManager::compileVertexShader() {
    compileShaderHelper(vertexShader, vertexShaderSource, GL_VERTEX_SHADER);
}

void ShaderManager::compileFragmentShader() {
    compileShaderHelper(fragmentShader, fragmentShaderSource, GL_FRAGMENT_SHADER);
}

void ShaderManager::compileLinesGeometryShader() {
    compileShaderHelper(geometryLinesShader, geometryLinesShaderSource, GL_GEOMETRY_SHADER);
}

void ShaderManager::compilePointsGeometryShader() {
    compileShaderHelper(geometryPointsShader, geometryPointsShaderSource, GL_GEOMETRY_SHADER);
}

void ShaderManager::compileComputeShaders() {
    compileShaderHelper(computeShader, computeShaderSource, GL_COMPUTE_SHADER);
}

void createProgramHelper(GLuint& program, GLuint shaders[]) {
    GLint linkSuccess = 0;
    GLchar infoLog[512];
    program = glCreateProgram();

    // Attach all shaders to the program
    while (*shaders) {
        glAttachShader(program, *shaders);
        shaders++;
    }

    // Link the program
    glBindAttribLocation(program, 0, "vPosition");
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linkSuccess);

    // Check for linking errors
    if (!linkSuccess) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        LOGE("shaderManager", "ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s", infoLog);
    }
}

void ShaderManager::createLinesProgram() {
    createProgramHelper(shaderLinesProgram, (GLuint[]) {vertexShader, geometryLinesShader, fragmentShader, 0});
}

void ShaderManager::createPointsProgram() {
    createProgramHelper(shaderPointsProgram, (GLuint[]) {vertexShader, geometryPointsShader, fragmentShader, 0});
}

void ShaderManager::createShaderProgram() {
    createProgramHelper(shaderComputeProgram, (GLuint[]) {computeShader, 0});
}

void ShaderManager::detachShaders() {
    glDetachShader(shaderLinesProgram, vertexShader);
    glDetachShader(shaderLinesProgram, geometryLinesShader);
    glDetachShader(shaderLinesProgram, fragmentShader);
    glDetachShader(shaderPointsProgram, vertexShader);
    glDetachShader(shaderPointsProgram, geometryPointsShader);
    glDetachShader(shaderPointsProgram, fragmentShader);
    glDetachShader(shaderComputeProgram, computeShader);
}

void ShaderManager::deleteShaders() {
    glDeleteShader(vertexShader);
    glDeleteShader(geometryPointsShader);
    glDeleteShader(geometryLinesShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(computeShader);
}


void ShaderManager::compileAndLinkShaders() {
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


void ShaderManager::loadShaderSources() {
    vertexShaderSource = loadShaderFile("vertex_shader.glsl");
    fragmentShaderSource = loadShaderFile("fragment_shader.glsl");
    geometryLinesShaderSource = loadShaderFile("geometry_lines_shader.glsl");
    geometryPointsShaderSource = loadShaderFile("geometry_points_shader.glsl");
    computeShaderSource = loadShaderFile("compute_shader.glsl");
}

void ShaderManager::checkShaderProgramLinkStatus() {
    // Check if the shader program linked successfully
    GLint linked;
    glGetProgramiv(shaderLinesProgram, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLchar linkLog[1024];
        glGetProgramInfoLog(shaderLinesProgram, sizeof(linkLog), NULL, linkLog);
        LOGE("shaderManager", "Shader Program Link Error: %s", linkLog);
    }

    // Check if the shader program linked successfully
    GLint linkedPoints;
    glGetProgramiv(shaderPointsProgram, GL_LINK_STATUS, &linkedPoints);
    if (!linkedPoints) {
        GLchar linkLog[1024];
        glGetProgramInfoLog(shaderPointsProgram, sizeof(linkLog), NULL, linkLog);
        LOGE("shaderManager", "Shader Program Link Error: %s", linkLog);
    }

    // Check for any other OpenGL errors
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        LOGE("shaderManager", "OpenGL setup error: %x", err);
    }
}

void ShaderManager::cleanShaderSources() {
    vertexShaderSource.clear();
    fragmentShaderSource.clear();
    geometryLinesShaderSource.clear();
    geometryPointsShaderSource.clear();
    computeShaderSource.clear();
}

void ShaderManager::createShaderPrograms() {
    loadShaderSources();
    compileAndLinkShaders();
    checkShaderProgramLinkStatus();
    cleanShaderSources();
}
