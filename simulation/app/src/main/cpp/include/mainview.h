//
// Created by martin on 08-05-2024.
//


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
#include "shaderManager.h"
#include "navig_cube.h"
#include "outline_box.h"


/**
 * @class Mainview
 * @brief This class provides the main view for the application.
 */
class Mainview {
public:
    /**
     * @brief Constructor
     *
     * @param assetManager A pointer to the asset manager.
     */
    Mainview(AAssetManager* assetManager);

    /**
     * @brief Destructor
     */
    ~Mainview();

    /**
     * @brief Sets the frame drawing
     */
    void setFrame();

    /**
     * @brief Initial graphics setup
     */
    void setupGraphics();

    /**
     * @brief Loads new vector field data.

     * @param vertices A reference to the flat vector of vertices (6 floats per vector).
     */
    void loadVectorFieldData(std::vector<float>& verticesOld, std::vector<float>& verticesNew);

    /**
     * @brief Draws the vector field.
     *
     * @param size Number of vectors to draw.
     */
    void drawVectorField(int size);

    /**
     * @brief Creates a buffer for the vector field.
     *
     * @param vertices A reference to the flat vector of vertices (6 floats per vector).
     */
    void createVectorFieldBuffer(std::vector<float>& vertices);

    /**
     * @brief Creates a buffer for the particles.
     *
     * @param particlesPos A reference to the flat vector of particle positions (3 floats per particle).
     */
    void createParticlesBuffer(std::vector<float>& particlesPos);

    /**
     * @brief Loads particle data.
     *
     * @param particlesPos A reference to the flat vector of particle positions.
     */
    void loadParticlesData(std::vector<float>& particlesPos);

    /**
     * @brief Draws the particles.
     *
     * @param size Number of particles to draw.
     */
    void drawParticles(int size);

    /**
     * @brief Creates a compute buffer.
     *
     * @param vector_field0 A reference to the previous vector field.
     * @param vector_field1 A reference to the next vector field.
     * @param vector_field2 A reference to the vector field to be loaded next.
     */
    void createComputeBuffer(std::vector<float>& vector_field0, std::vector<float>& vector_field1, std::vector<float>& vector_field2);

    /**
     * @brief Loads constant uniforms.
     *
     * @param dt Simulation he time step.
     * @param width The width simulation dimension.
     * @param height The height simulation dimension.
     * @param depth The depth simulation dimension.
     */
    void loadConstUniforms(float dt, int width, int height, int depth);

    /**
     * @brief Loads a compute buffer (not used for rendering) with new data so that it can be
     * efficiently swapped when required.
     *
     * @param vector_field New vector field vertices to load.
     * @param globalFence A reference to the global fence.
     */
    void preloadComputeBuffer(std::vector<float>& vector_field, std::atomic<GLsync>& globalFence);

    /**
     * @brief Swaps the compute buffers. prev <- next, next <- future.
     * @note This function is O(1).
     */
    void loadComputeBuffer();

    /**
     * @brief Dispatches the compute shader to update the particle positions.
     */
    void dispatchComputeShader();

    /**
     * @brief Getter for the object defining the view transformations.
     *
     * @return A reference to the transforms object.
     */
    Transforms& getTransforms() { return *transforms; }

    /**
     * @brief Draws the UI elements
     */
    void drawUI();

    void setMoveAttribX(float perc) { move_attrib_x = (int)(perc * grid_width); }
    void setMoveAttribY(float perc) { move_attrib_y = (int)(perc * grid_height); }

private:
    /**
     * @brief Loads the uniforms.
     */
    void loadUniforms();

    ShaderManager *shaderManager;
    Transforms *transforms;
    NavigCube *navigCube;
    OutlineBox *outlineBox;


    // Uniforms
    GLint pointSize;
    GLint modelLocationBox;
    GLint viewLocationBox;
    GLint projectionLocationBox;
    GLint modelLocationPoints;
    GLint viewLocationPoints;
    GLint projectionLocationPoints;
    GLint globalTimeInStepLocation;

    // Buffers
    GLuint particleVBO;
    GLuint particleVAO;
    GLuint vectorFieldVBO;
    GLuint vectorFieldVAO;
    GLuint vectorFieldEBO;
    GLuint computeVectorField0SSBO;
    GLuint computeVectorField1SSBO;
    GLuint computeVectorField2SSBO;


    // Array containing vertices for the triangles forming the faces of the cuboid
    std::vector<float> faceTriangles;
    std::vector<unsigned int> faceTriangleIndices;

    // Faces moving
    int move_attrib_x = 0;  // [0, width)
    int move_attrib_y = 0;  // [0, height)
    int move_attrib_z = 0;  // [0, depth)
};

#endif // GL_SHADER_MANAGER_H
