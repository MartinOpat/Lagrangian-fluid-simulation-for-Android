//
// Created by martin on 5/28/24.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_TRANSFORMS_H
#define LAGRANGIAN_FLUID_SIMULATION_TRANSFORMS_H

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "consts.h"
#include "algorithm"

class Transforms {
public:
    Transforms();

    void setRotation(float rotateX, float rotateY, float rotateZ);
    void setScale(float scale);
    void setAspectRatio(float aspectRatio);

    glm::vec3 getRotation() { return rotation; }
    float getScale() { return scale; }



    glm::mat4 modelTransform;
    glm::mat4 projectionTransform;
    glm::mat4 viewTransform;

private:
    void updateTransformations();

    glm::vec3 rotation;
    float scale;
    float aspectRatio;
};

#endif //LAGRANGIAN_FLUID_SIMULATION_TRANSFORMS_H
