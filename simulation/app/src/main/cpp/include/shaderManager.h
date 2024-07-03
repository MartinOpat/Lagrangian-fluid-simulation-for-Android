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

/**
 * @class ShaderManager
 * @brief This class initializes the shaders used in the application.
 */
class ShaderManager {
public:
    /**
     * @brief Constructor.
     *
     * @param assetManager A pointer to the asset manager.
     */
    ShaderManager(AAssetManager* assetManager);

    /**
     * @brief Destructor.
     */
    ~ShaderManager();

    // Shader programs
    GLuint shaderBoxProgram;
    GLuint shaderPointsProgram;
    GLuint shaderComputeProgram;
    GLuint shaderUIProgram;

    /**
     * @brief Creates the shader programs.
     */
    void createShaderPrograms();

private:
    AAssetManager *assetManager;

    /**
     * @brief Loads a shader file with the given filename.
     *
     * @param fileName The name of the file to load.
     * @return The contents of the shader file as a string.
     */
    std::string loadShaderFile(const char* fileName);

    /**
     * @brief Compiles and links the shaders.
     */
    void compileAndLinkShaders();

    /**
     * @brief Loads the shader sources.
     */
    void loadShaderSources();

    /**
     * @brief Checks the link status of the shader program.
     */
    void checkShaderProgramLinkStatus();

    /**
     * @brief Compiles the vertex shader.
     */
    void compileVertexShaders();

    /**
     * @brief Compiles the fragment shader.
     */
    void compileFragmentShaders();

    /**
     * @brief Compiles the geometry shader for lines (vector field).
     */
    void compileGeometryShaders();

    /**
     * @brief Compiles the compute shaders.
     */
    void compileComputeShaders();

    /**
     * @brief Creates the shader program for lines (vector field).
     */
    void createLinesProgram();

    /**
     * @brief Creates the shader program for points (particles).
     */
    void createPointsProgram();

    /**
     * @brief Creates the compute shader program.
     */
    void createComputeProgram();

    /**
     * @brief Creates the UI shader program.
     */
    void createUIProgram();

    /**
     * @brief Detaches the shaders.
     */
    void detachShaders();

    /**
     * @brief Deletes the shaders.
     */
    void deleteShaders();

    /**
     * @brief Cleans the shader sources strings.
     */
    void cleanShaderSources();

    // Shader sources
    std::string boxVertexShaderSource;
    std::string fragmentShaderLinesSource;
    std::string fragmentShaderPointsSource;
    std::string geometryLinesShaderSource;
    std::string geometryPointsShaderSource;
    std::string computeShaderSource;
    std::string uiVertexShaderSource;
    std::string uiFragmentShaderSource;

    // Shaders
    GLuint vertexShader;
    GLuint fragmentShaderLines;
    GLuint fragmentShaderPoints;
    GLuint geometryLinesShader;
    GLuint geometryPointsShader;
    GLuint computeShader;
    GLuint uiVertexShader;
    GLuint uiFragmentShader;

};

#endif //LAGRANGIAN_FLUID_SIMULATION_SHADERMANAGER_H
