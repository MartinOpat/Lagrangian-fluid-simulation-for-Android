//
// Created by martin on 5/28/24.
//

#include "transforms.h"

Transforms::Transforms() : scale(1.0f), rotation(glm::vec3(0.0f, 0.0f, 0.0f)) {
    modelTransform = glm::identity<glm::mat4>();
    projectionTransform = glm::identity<glm::mat4>();
    float max_dim = std::max(FIELD_WIDTH, std::max(FIELD_HEIGHT, FIELD_DEPTH));
    projectionTransform = glm::ortho(-max_dim, max_dim, -max_dim, max_dim, -max_dim, max_dim);
    viewTransform = glm::identity<glm::mat4>();
    setRotation(0.0f, 0.0f, M_PI/2.0f);
    setScale(0.5f);
    updateTransformations();
}


void Transforms::setRotation(float rotateX, float rotateY, float rotateZ) {
    this->rotation = glm::vec3(rotateX, rotateY, rotateZ);
    updateTransformations();
}

void Transforms::setScale(float scale) {
    this->scale = scale;
    updateTransformations();
}

void Transforms::updateTransformations() {
    modelTransform = glm::identity<glm::mat4>();
    modelTransform = glm::scale(modelTransform, glm::vec3(scale, scale, scale));

    // Rotation behaviour is order dependent
    modelTransform *= glm::rotate(glm::identity<glm::mat4>(), rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    modelTransform *= glm::rotate(glm::identity<glm::mat4>(), -rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    modelTransform *= glm::rotate(glm::identity<glm::mat4>(), rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
}