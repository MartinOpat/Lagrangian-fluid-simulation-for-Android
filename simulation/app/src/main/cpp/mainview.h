#ifndef GL_SHADER_MANAGER_H
#define GL_SHADER_MANAGER_H

#include <string>
#include <chrono>
#include <GLES3/gl3.h>
#include <android/asset_manager.h>

#include "android_logging.h"
#include "png_loader.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"



class GLShaderManager {
public:
    GLShaderManager(AAssetManager* assetManager);
    ~GLShaderManager();

    std::string loadShaderFile(const char* fileName);
    void createTexture(const ImageData& texData);
    void compileAndLinkShaders();
    void setFrame();
    void setupGraphics();

    void loadVectorFieldData(std::vector<float> vertices);
    void drawVectorField(int size);
    void createVectorFieldBuffer(std::vector<float> vertices);

    void createParticlesBuffer(std::vector<float> particlesPos);
    void loadParticlesData(std::vector<float> particlesPos);
    void drawParticles(int size);

    void setRotation(float rotateX, float rotateY, float rotateZ);
    void setScale(float scale);
    void updateTransformations();

    glm::vec3 getRotation() { return rotation; }
    float getScale() { return scale; }

    GLuint shaderProgram;
    std::chrono::steady_clock::time_point startTime;

private:
    AAssetManager* assetManager;
    GLuint vertexShader, fragmentShader;
    GLuint textureID;

    std::string vertexShaderSource;
    std::string fragmentShaderSource;

    // Uniforms
    GLint isPointLocation;
    GLfloat pointSize;  // TODO: This can (and probably should) be a GLint
    GLint modelLocation;
    GLint viewLocation;
    GLint projectionLocation;


    // Buffers
    GLuint particleVBO;
    GLuint particleVAO;

    GLuint vectorFieldVBO;
    GLuint vectorFieldVAO;

    // Transformations
    float scale;
    glm::vec3 rotation;
    glm::mat4 modelTransform;
    glm::mat4 projectionTransform;
    glm::mat4 viewTransform;
};

#endif // GL_SHADER_MANAGER_H
