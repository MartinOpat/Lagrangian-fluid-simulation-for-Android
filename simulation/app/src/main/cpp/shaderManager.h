//
// Created by martin on 01-06-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_SHADERMANAGER_H
#define LAGRANGIAN_FLUID_SIMULATION_SHADERMANAGER_H

#include <GLES3/gl32.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <string>
#include <android/asset_manager.h>


#include "android_logging.h"

class ShaderManager {
public:

    ShaderManager(AAssetManager* assetManager);
    ~ShaderManager();

    GLuint shaderLinesProgram;
    GLuint shaderPointsProgram;
    GLuint shaderComputeProgram;

    void createShaderPrograms();

private:
    AAssetManager *assetManager;

    std::string loadShaderFile(const char* fileName);

    void compileAndLinkShaders();
    void loadShaderSources();
    void checkShaderProgramLinkStatus();
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
    void cleanShaderSources();

    std::string vertexShaderSource;
    std::string fragmentShaderSource;
    std::string geometryLinesShaderSource;
    std::string geometryPointsShaderSource;
    std::string computeShaderSource;

    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint geometryLinesShader;
    GLuint geometryPointsShader;
    GLuint computeShader;

};

#endif //LAGRANGIAN_FLUID_SIMULATION_SHADERMANAGER_H
