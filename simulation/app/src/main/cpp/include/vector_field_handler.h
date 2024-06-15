//
// Created by martin on 08-05-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_VECTOR_FIELD_HANDLER_H
#define LAGRANGIAN_FLUID_SIMULATION_VECTOR_FIELD_HANDLER_H

#include "mainview.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "android_logging.h"
#include "netcdf_reader.h"
#include "consts.h"
#include <vector>

/**
 * @class VectorFieldHandler
 * @brief This class handles the vector field in the application.
 */
class VectorFieldHandler {
public:
    /**
     * @brief Constructor for the VectorFieldHandler class.
     *
     * @param finenessXY The fineness of the vector field in the X and Y axes.
     * @param finenessZ The fineness of the vector field in the Z-axis.
     */
    VectorFieldHandler(int finenessXY = 1, int finenessZ = 1);

    /**
     * @brief Gets the velocity field at the given position.
     *
     * @param position The position at which to calculate the velocity field.
     * @param velocity The var. to store the  velocity field's value.
     */
    void velocityField(const glm::vec3 &position, glm::vec3 &velocity);


    /**
     * @brief Prepares the vertex data with the given u, v, and w data.
     *
     * @param uData The u data.
     * @param vData The v data.
     * @param wData The w data.
     */
    void prepareVertexData(const std::vector<float>& uData, const std::vector<float>& vData, const std::vector<float>& wData);


    /**
     * @brief Helper method for loading a time step with the given file paths for u, v, and w data.
     *
     * @param fileUPath The file path for the u data.
     * @param fileVPath The file path for the v data.
     * @param fileWPath The file path for the w data.
     */
    void loadTimeStepHelper(const std::string& fileUPath, const std::string& fileVPath, const std::string& fileWPath);

    /**
     * @brief Loads a time step with the given file descriptors for u, v, and w data.
     *
     * @param fdU The file descriptor for the u data.
     * @param fdV The file descriptor for the v data.
     * @param fdW The file descriptor for the w data.
     */
    void loadTimeStep(int fdU, int fdV, int fdW);

    /**
     * @brief Updates the time step.
     */
    void updateTimeStep();

    /**
     * @brief Draws the vector field with the view.
     *
     * @param mainview The view
     */
    void draw(Mainview& mainview);

    /**
     * @brief Getter for the previous time step vertices.
     *
     * @return The previous vertices.
     */
    std::vector<float>& getOldVertices() {return allVertices[0];};
//    std::vector<float>& getOldVertices() {return displayVertices[0];};

    /**
     * @brief Getter for the next time step vertices.
     *
     * @return The next vertices.
     */
    std::vector<float>& getNewVertices() {return allVertices[1];};
//    std::vector<float>& getNewVertices() {return displayVertices[1];};

    /**
     * @brief Getter for the next (unloaded) time step vertices.
     *
     * @return The future vertices.
     */
    std::vector<float>& getFutureVertices() {return allVertices[2];};
//    std::vector<float>& getFutureVertices() {return displayVertices[2];};

    /**
     * @brief Getter for the width.
     *
     * @return The width.
     */
    int getWidth() {return width;};
//    int getWidth() {return (width + finenessXY - 1) / finenessXY;};

    /**
     * @brief Getter for the height.
     *
     * @return The height.
     */
    int getHeight() {return height;};
//    int getHeight() {return (height + finenessXY - 1) / finenessXY;};

    /**
     * @brief Getter for the depth.
     *
     * @return The depth.
     */
    int getDepth() {return depth ;};
//    int getDepth() {return (depth + finenessZ - 1) / finenessZ;};

private:
    // Dimensions of the loaded vector field
    int width ;
    int height;
    int depth ;

    // Defines how many vertices to omit for rendering (higher value = less vertices)
    int finenessXY;
    int finenessZ;

    std::vector<std::vector<float>> allVertices;
    std::vector<std::vector<float>> displayVertices;

};

#endif //LAGRANGIAN_FLUID_SIMULATION_VECTOR_FIELD_HANDLER_H
