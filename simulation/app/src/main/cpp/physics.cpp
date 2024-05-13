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

// args can contain any arguments, but at least the position
glm::vec3 Physics::dvdt(std::vector<glm::vec3> args) {
    glm::vec3 velField;

    if (args.size() < 1) {
        LOGE("Physics::dvdt: args must contain at least the position of the particle");
        return glm::vec3(0.0f);
    }

    glm::vec3 pos = args[0];

    vectorFieldHandler.velocityField(pos, velField); // Fluid velocity at particle's position
    switch (model) {
        case Model::particles_simple: {
            glm::vec3 vel = args[1];
            return - b / m * (vel - velField);
        }

        case Model::particles: {
            glm::vec3 vel = args[1];
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

        case Model::particles_advection: {
            return velField;
        }
    }
}

void Physics::eulerStep(Particle& particle) {
    particle.acceleration = dvdt(particle.position, particle.velocity);
    particle.velocity += particle.acceleration * dt;
    particle.position += particle.velocity * dt;
}

void Physics::rk4Step(Particle& particle) {
    // Define k1, k2, k3, k4 for position and velocity
    glm::vec3 a1, a2, a3, a4;
    glm::vec3 v1, v2, v3, v4;

    // Initial adjusted velocity
    a1 = dvdt({particle.position, particle.velocity, particle.acceleration});
    v1 = particle.velocity;

    // Update for k2
    a2 = dvdt({particle.position + 0.5f * v1 * dt, particle.velocity + 0.5f * a1 * dt, particle.acceleration});
    v2 = particle.velocity + 0.5f * a1 * dt;

    // Update for k3
    a3 = dvdt({particle.position + 0.5f * v2 * dt, particle.velocity + 0.5f * a2 * dt, particle.acceleration});
    v3 = particle.velocity + 0.5f * a2 * dt;

    // Update for k4
    a4 = dvdt({particle.position + v3 * dt, particle.velocity + a3 * dt, particle.acceleration});
    v4 = particle.velocity + a3 * dt;

    particle.acceleration = (a1 + 2.0f * a2 + 2.0f * a3 + a4) / 6.0f;
    particle.velocity += dt * (a1 + 2.0f * a2 + 2.0f * a3 + a4) / 6.0f;
    particle.position += dt * (v1 + 2.0f * v2 + 2.0f * v3 + v4) / 6.0f;
}


void Physics::advectionStep(Particle &particle) {
    glm::vec3 v1 = dvdt({particle.position});
    glm::vec3 pos1 = particle.position + 0.5f * v1 * dt;
    glm::vec3 v2 = dvdt({pos1});
    glm::vec3 pos2 = particle.position + 0.5f * v2 * dt;
    glm::vec3 v3 = dvdt({pos2});
    glm::vec3 pos3 = particle.position + v3 * dt;
    glm::vec3 v4 = dvdt({pos3});

    particle.position += dt * (v1 + 2.0f * v2 + 2.0f * v3 + v4) / 6.0f;
    LOGI("Physics::advectionStep: particle position: %f, %f, %f", particle.position.x, particle.position.y, particle.position.z);
}

void Physics::doStep(Particle& particle) {
    if (model == Model::particles_advection) {
        advectionStep(particle);
    } else {
        rk4Step(particle);
    }
}






