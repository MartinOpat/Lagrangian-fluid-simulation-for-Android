//
// Created by martin on 12-05-2024.
//

#include "physics.h"


Physics::Physics(VectorFieldHandler vectorFieldHandler, Physics::Model model): vectorFieldHandler(vectorFieldHandler), model(model) {}

glm::vec3 Physics::dvdt(glm::vec3 pos, glm::vec3 vel) {
    glm::vec3 velField;
    vectorFieldHandler.velocityField(pos, velField); // Fluid velocity at particle's position
    return - b / m * (vel - velField);
}

// args can contain any arguments, but at leat the position and velocity of the particle
glm::vec3 Physics::dvdt(std::vector<glm::vec3> args) {
    glm::vec3 velField;

    if (args.size() < 2) {
        LOGE("Physics::dvdt: args must contain at least the position and velocity of the particle");
        return glm::vec3(0.0f);
    }

    glm::vec3 pos = args[0];
    glm::vec3 vel = args[1];

    vectorFieldHandler.velocityField(pos, velField); // Fluid velocity at particle's position
    switch (model) {
        case Model::particles_simple:
            return - b / m * (vel - velField);
        case Model::particles: {
            glm::vec3 acc = args[2];
            glm::vec3 Fd = - b * (vel - velField);
            glm::vec3 Fc;
            if (std::abs(acc.x) < 0.0001f && std::abs(acc.y) < 0.0001f && std::abs(acc.z) < 0.0001f) {
                Fc = glm::vec3(0.0f);
            } else if (std::abs(vel.x) < 0.0001f && std::abs(vel.y) < 0.0001f && std::abs(vel.z) < 0.0001f) {
                Fc = glm::vec3(0.0f);
            } else {
                float R = std::pow(glm::length(vel), 3) / glm::length(glm::cross(vel, acc));
                Fc = m * glm::dot(vel, vel) / R * glm::normalize(acc);  // + -> inwards
            }
            glm::vec3 Fb = rho * V * g * glm::vec3(0.0f, 0.0f, 1.0f);
            glm::vec3 Fg = m * g * glm::vec3(0.0f, 0.0f, -1.0f);
            glm::vec3 Fm = -C * rho * V * acc;
            return (Fd + Fc + Fb + Fg + Fm) / m;
        }
        case Model::particles_advection:
            return - b / m * (vel - velField);
        case Model::density_advection:
            return - b / m * (vel - velField);
    }
}



