#ifndef GL_SHADER_MANAGER_H
#define GL_SHADER_MANAGER_H

#include <string>
#include <chrono>
//#include <GLES2/gl2.h>
#include <GLES3/gl3.h>
#include <android/asset_manager.h>

#include "android_logging.h"
#include "png_loader.h"
#include "particle.h"



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

    void loadVectorFieldData(std::vector<float> vertices);
    void drawVectorField(int size);
    void createVectorFieldBuffer(std::vector<float> vertices);

    void createParticlesBuffer(std::vector<float> particlesPos);
    void loadParticlesData(std::vector<float> particlesPos);
    void drawParticles(int size);

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
    GLuint particleVAO;

    GLint isPointLocation;

    GLuint vectorFieldVBO;
    GLuint vectorFieldVAO;
};

#endif // GL_SHADER_MANAGER_H
