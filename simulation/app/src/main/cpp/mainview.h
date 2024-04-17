#ifndef GL_SHADER_MANAGER_H
#define GL_SHADER_MANAGER_H

#include <string>
#include <chrono>
#include <GLES2/gl2.h>
#include <android/asset_manager.h>
#include "png_loader.h"

class GLShaderManager {
public:
    GLShaderManager(AAssetManager* assetManager);
    ~GLShaderManager();

    std::string loadShaderFile(const char* fileName);
    void createTexture(const ImageData& texData);
    void compileAndLinkShaders();
    void setFrame();
    void drawTriangle();
    void drawParticle();
    void setupGraphics();

    GLuint shaderProgram;
    std::chrono::steady_clock::time_point startTime;

private:
    AAssetManager* assetManager;
    GLuint vertexShader, fragmentShader;
    GLuint textureID;

    std::string vertexShaderSource;
    std::string fragmentShaderSource;

    GLuint triangleVBO;
    GLuint particleVBO;

    GLint isPointLocation;
};

#endif // GL_SHADER_MANAGER_H
