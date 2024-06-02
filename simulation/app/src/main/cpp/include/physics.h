//
// Created by martin on 12-05-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_PHYSICS_H
#define LAGRANGIAN_FLUID_SIMULATION_PHYSICS_H

#include "glm/glm.hpp"
#include "vector_field_handler.h"
#include "particle.h"

/**
 * @class Physics
 * @brief This class handles the physics of a particle simulation.
 */
class Physics {
public:
    /**
     * @enum Model
     * @brief The model of physics for the particles.
     */
    enum class Model {
        particles_simple,       // drag force
        particles,              // centripetal, buoyant, drag, gravity, drag, and added mass force
        particles_advection,    // Advection equation for particles - default
    };

    /**
     * @brief Constructor.
     *
     * @param vectorFieldHandler The vector field handler.
     * @param model The model of physics for the particles.
     */
    Physics(VectorFieldHandler& vectorFieldHandler, Model model = Model::particles_advection);

    /**
     * @brief Calculates the derivative of the simulated quantity.
     * For advection: returns the fluid velocity at the particle's position.
     * For other models: returns the acceleration of the particle.
     *
     * @param pos The position of the particle.
     * @param vel The velocity of the particle.
     * @return The quantity are defined above.
     */
    glm::vec3 dvdt(glm::vec3 pos, glm::vec3 vel);

    /**
     * @brief Calculates the derivative of the simulated quantity.
     * For advection: returns the fluid velocity at the particle's position.
     * For other models: returns the acceleration of the particle.
     *
     * @param args The arguments for the calculation (at least particle position).
     * @return The quantity are defined above.
     */
    glm::vec3 dvdt(std::vector<glm::vec3> args);

    /**
     * @brief Performs an Euler integration step.
     *
     * @param particle The particle to update.
     */
    void eulerStep(Particle& particle); // Euler integration - mostly debug purposes

    /**
     * @brief Performs a Runge-Kutta 4 integration step.
     *
     * @param particle The particle to update.
     */
    void rk4Step(Particle& particle);

    /**
     * @brief Performs an advection step.
     *
     * @param particle The particle to update.
     */
    void advectionStep(Particle& particle);

    /**
     * @brief Performs a step of the simulation.
     *
     * @param particle The particle to update.
     */
    void doStep(Particle& particle);

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
