#ifndef GL_SHADER_MANAGER_H
#define GL_SHADER_MANAGER_H

#include <string>
#include <chrono>
#include <GLES3/gl32.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <android/asset_manager.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <atomic>

#include "android_logging.h"
#include "consts.h"
#include "transforms.h"



class Mainview {
public:
    Mainview(AAssetManager* assetManager);
    ~Mainview();

    void compileAndLinkShaders();
    void setFrame();
    void setupGraphics();

    void loadVectorFieldData(std::vector<float>& vertices);
    void drawVectorField(int size);
    void createVectorFieldBuffer(std::vector<float>& vertices);

    void createParticlesBuffer(std::vector<float>& particlesPos);
    void loadParticlesData(std::vector<float>& particlesPos);
    void drawParticles(int size);

    void createComputeBuffer(std::vector<float>& vector_field0, std::vector<float>& vector_field1, std::vector<float>& vector_field2);
    void preloadComputeBuffer(std::vector<float>& vector_field, std::atomic<GLsync>& globalFence);
    void loadComputeBuffer();
    void dispatchComputeShader(float dt, float global_time_in_step, int width, int height, int depth);

    Transforms& getTransforms() { return *transforms; }

    GLuint shaderLinesProgram;
    GLuint shaderPointsProgram;
    GLuint shaderComputeProgram;
    std::chrono::steady_clock::time_point startTime;

private:
    std::string loadShaderFile(const char* fileName);

    void compileVertexShader();
    void compileFragmentShader();
    void compileLinesGeometryShader();
    void compilePointsGeometryShader();
    void compileComputeShaders();
    void createLinesProgram();
    void createPointsProgram();
    void createShaderProgram();
    void detachShaders();
    void deleteShaders();

    AAssetManager* assetManager;
    GLuint vertexShader, fragmentShader, geometryLinesShader, geometryPointsShader, computeShader;

    std::string vertexShaderSource;
    std::string fragmentShaderSource;
    std::string geometryLinesShaderSource;
    std::string geometryPointsShaderSource;
    std::string computeShaderSource;

    // Uniforms
    GLint isPointLocationLines;
    GLint isPointLocationPoints;
    GLfloat pointSize;  // TODO: This can (and probably should) be a GLint
    GLint modelLocationLines;
    GLint viewLocationLines;
    GLint projectionLocationLines;
    GLint modelLocationPoints;
    GLint viewLocationPoints;
    GLint projectionLocationPoints;

    // Buffers
    GLuint particleVBO;
    GLuint particleVAO;

    GLuint vectorFieldVBO;
    GLuint vectorFieldVAO;

    GLuint computeVectorField0SSBO;
    GLuint computeVectorField1SSBO;
    GLuint computeVectorField2SSBO;

    // Transforms
    Transforms *transforms;
};

#endif // GL_SHADER_MANAGER_H
