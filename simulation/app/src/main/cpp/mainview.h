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

#include "android_logging.h"
#include "png_loader.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "consts.h"



class Mainview {
public:
    Mainview(AAssetManager* assetManager);
    ~Mainview();

    std::string loadShaderFile(const char* fileName);
    void compileAndLinkShaders();
    void setFrame();
    void setupGraphics();

    void loadVectorFieldData(std::vector<float>& vertices);
    void drawVectorField(int size);
    void createVectorFieldBuffer(std::vector<float>& vertices);

    void createParticlesBuffer(std::vector<float>& particlesPos);
    void loadParticlesData(std::vector<float>& particlesPos);
    void drawParticles(int size);

    void setRotation(float rotateX, float rotateY, float rotateZ);
    void setScale(float scale);
    void updateTransformations();

    glm::vec3 getRotation() { return rotation; }
    float getScale() { return scale; }

    void createComputeBuffer(std::vector<float>& vector_field_vertices);
    void loadComputeBuffer(std::vector<float>& vector_field0, std::vector<float>& vector_field1);
    void dispatchComputeShader(float dt, float global_time_in_step, int width, int height, int depth);

    GLuint shaderLinesProgram;
    GLuint shaderPointsProgram;
    GLuint shaderComputeProgram;
    std::chrono::steady_clock::time_point startTime;

private:
    AAssetManager* assetManager;
    GLuint vertexShader, fragmentShader, geometryLinesShader, geometryPointsShader, computeShader;
    GLuint textureID;

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

    // Transformations
    float scale;
    glm::vec3 rotation;
    glm::mat4 modelTransform;
    glm::mat4 projectionTransform;
    glm::mat4 viewTransform;
};

#endif // GL_SHADER_MANAGER_H
