//
// Created by martin on 12-05-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_PHYSICS_H
#define LAGRANGIAN_FLUID_SIMULATION_PHYSICS_H

#include "glm/glm.hpp"
#include "vector_field_handler.h"

class Physics {
public:
    enum class Model {
        particles_simple,       // centripetal, and drag force
        particles,              // centripetal, buoyant, drag, gravity, drag, and added mass force
        particles_advection,    // Advection equation for particles
        density_advection       // Advection equation for density
    };

    Physics(VectorFieldHandler vectorFieldHandler, Model model = Model::particles_simple);
    glm::vec3 dvdt(glm::vec3 pos, glm::vec3 vel);

    float dt = 0.02f;  // Time step
    float b = 0.8f;  // Drag coefficient
    float m = 1.0f;  // Mass of the particle

private:
    VectorFieldHandler vectorFieldHandler;
    Model model;
};

#endif //LAGRANGIAN_FLUID_SIMULATION_PHYSICS_H
