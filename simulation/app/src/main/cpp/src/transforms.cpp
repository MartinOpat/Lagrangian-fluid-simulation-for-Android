//
// Created by martin on 5/28/24.
//

#include "include/transforms.h"

Transforms::Transforms() : scale(1.0f), rotation(glm::vec3(0.0f, 0.0f, 0.0f)) {
    modelTransform = glm::identity<glm::mat4>();

    projectionTransform = glm::identity<glm::mat4>();
    float max_dim = std::max(FIELD_WIDTH, std::max(FIELD_HEIGHT, FIELD_DEPTH));
    projectionTransform = glm::ortho(-max_dim, max_dim, -max_dim, max_dim, -NEAR_FAR, NEAR_FAR);

    viewTransform = glm::identity<glm::mat4>();

    setRotation(0.0f, 0.0f, M_PI/2.0f);
    setScale(0.7f);
    setAspectRatio(0.5f);

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

void Transforms::setAspectRatio(float aspectRatio) {
    this->aspectRatio = aspectRatio;
    updateTransformations();
}

void Transforms::updateTransformations() {
    // Update model matrix
    modelTransform = glm::identity<glm::mat4>();
    modelTransform = glm::scale(modelTransform, glm::vec3(scale, scale, scale));

    // Rotation behaviour is order dependent
    modelTransform *= glm::rotate(glm::identity<glm::mat4>(), rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    modelTransform *= glm::rotate(glm::identity<glm::mat4>(), -rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    modelTransform *= glm::rotate(glm::identity<glm::mat4>(), rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));


    // Update the projection matrix according to the new aspect ratio
    float width = FIELD_WIDTH / 0.5f;
    float height = FIELD_HEIGHT;
    if (aspectRatio > 0.5f) {
        width *= aspectRatio;
    } else {
        width /= aspectRatio;
    }
    projectionTransform = glm::ortho(-width, width, -height, height, -NEAR_FAR, NEAR_FAR);
}