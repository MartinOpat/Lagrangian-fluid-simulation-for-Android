//
// Created by martin on 12-05-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_PHYSICS_H
#define LAGRANGIAN_FLUID_SIMULATION_PHYSICS_H

#include "glm/glm.hpp"
#include "vector_field_handler.h"
#include "particle.h"

class Physics {
public:
    enum class Model {
        particles_simple,       // drag force
        particles,              // centripetal, buoyant, drag, gravity, drag, and added mass force
        particles_advection,    // Advection equation for particles
    };

    Physics(VectorFieldHandler& vectorFieldHandler, Model model = Model::particles_simple);

    glm::vec3 dvdt(glm::vec3 pos, glm::vec3 vel);
    glm::vec3 dvdt(std::vector<glm::vec3> args);

    void eulerStep(Particle& particle); // Euler integration - mostly debug purposes
    void rk4Step(Particle& particle);
    void rk4Step(std::vector<Particle>& particles);  // This will have parallelism
    void advectionStep(Particle& particle);
    void advectionStep(std::vector<Particle>& particles);  // This will have parallelism
    void doStep(Particle& particle);
    void doStep(std::vector<Particle>& particles);  // This will have parallelism

    float dt = 0.02f;  // Time step
    float b = 50;  // Drag coefficient (6*pi*mu*radius = 0.017 for water)
    float m = 1.0f;  // Mass of the particle
    float rho = 1.0f;  // Density of the fluid
    float V = 1.0f;  // Volume of the particle
    float g = 9.81f;  // Gravity
    float C = 0.5f;  // Displacement coefficient

private:
    VectorFieldHandler& vectorFieldHandler;
    Model model;
};

#endif //LAGRANGIAN_FLUID_SIMULATION_PHYSICS_H
