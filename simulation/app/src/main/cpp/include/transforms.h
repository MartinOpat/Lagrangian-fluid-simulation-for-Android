//
// Created by martin on 5/28/24.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_TRANSFORMS_H
#define LAGRANGIAN_FLUID_SIMULATION_TRANSFORMS_H

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "consts.h"
#include "algorithm"

/**
 * @class Transforms
 * @brief This class handles transformations in the application.
 */
class Transforms {
public:
    /**
     * @brief Constructor.
     */
    Transforms();

    /**
     * @brief Sets a new rotation trasformation.
     *
     * @param rotateX The rotation around the x-axis.
     * @param rotateY The rotation around the y-axis.
     * @param rotateZ The rotation around the z-axis.
     */
    void setRotation(float rotateX, float rotateY, float rotateZ);

    /**
     * @brief Sets a new scale transformation.
     *
     * @param scale The new scale.
     */
    void setScale(float scale);

    /**
     * @brief Sets a new aspect ratio.
     *
     * @param aspectRatio The new aspect ratio of the transformations.
     */
    void setAspectRatio(float aspectRatio);

    /**
     * @brief Getter for the rotation of the transformations.
     *
     * @return The rotation of the transformations as a glm::vec3.
     */
    glm::vec3 getRotation() { return rotation; }

    /**
     * @brief Getter for the scale of the transformations.
     *
     * @return The scale of the transformations as a float.
     */
    float getScale() { return scale; }

    // Transformation matrices
    glm::mat4 modelTransform;
    glm::mat4 projectionTransform;
    glm::mat4 viewTransform;

private:
    /**
     * @brief Utility function to apply the current transfromations to the model matrix.
     */
    void updateTransformations();


    glm::vec3 rotation;
    float scale;
    float aspectRatio;
};

#endif //LAGRANGIAN_FLUID_SIMULATION_TRANSFORMS_H
