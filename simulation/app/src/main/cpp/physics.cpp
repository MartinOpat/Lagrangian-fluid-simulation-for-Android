//
// Created by martin on 12-05-2024.
//

#include "physics.h"


Physics::Physics(VectorFieldHandler vectorFieldHandler, Physics::Model model): vectorFieldHandler(vectorFieldHandler), model(model) {}

glm::vec3 Physics::dvdt(glm::vec3 pos, glm::vec3 vel) {
    glm::vec3 velField;
    vectorFieldHandler.velocityField(pos, velField); // Fluid velocity at particle's position
    switch (model) {
        case Model::particles_simple:
            return - b * (vel - velField);
        case Model::particles:
            return - b * (vel - velField);
        case Model::particles_advection:
            return - b * (vel - velField);
        case Model::density_advection:
            return - b * (vel - velField);
    }
}

