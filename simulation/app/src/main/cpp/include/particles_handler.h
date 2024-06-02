//
// Created by martin on 08-05-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_PARTICLES_HANDLER_H
#define LAGRANGIAN_FLUID_SIMULATION_PARTICLES_HANDLER_H

#include "particle.h"
#include "mainview.h"
#include "physics.h"
#include "vector_field_handler.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ThreadPool.h"

#include <stdio.h>
#include <vector>
#include <algorithm>
#include <thread>

#include "netcdf_reader.h"

/**
 * @class ParticlesHandler
 * @brief This class handles the particles in a physics simulation.
 */
class ParticlesHandler {
public:
    /**
     * @enum InitType
     * @brief The type of initialization for the particles.
     */
    enum class InitType {
        line,       // A single diagonal line
        two_lines,      // Two diagonal lines
        explosion   // Explosion of particles
    };

    /**
     * @brief Constructor with initialization.
     *
     * @param type The type of initialization.
     * @param physics The physics object.
     * @param num The number of particles.
     */
    ParticlesHandler(InitType type, Physics& physics, int num = 100);  // Constructor with initialization

    /**
     * @brief Constructor without initialization (for loading from file).
     *
     * @param physics The physics object.
     * @param num The number of particles.
     */
    ParticlesHandler(Physics& physics, int num = 100);  // Constructor without initialization (for loading from file)

    /**
     * @brief Initializes the particles with the given type of initialization.
     *
     * @param type The type of initialization for the particles.
     */
    void initParticles(InitType type);

    /**
     * @brief Updates the particles.
     */
    void updateParticles();

    /**
     * @brief Updates the particles in parallel.
     */
    void updateParticlesParallel();

    /**
     * @brief Updates the particles using a thread pool.
     */
    void updateParticlesPool();

    /**
     * @brief Draws the particles.
     *
     * @param mainview A reference to the view to render in.
     */
    void drawParticles(Mainview& mainview);

    /**
     * @brief Getter for the positions of the particles.
     *
     * @return A reference to the vector of particle positions.
     */
    std::vector<float>& getParticlesPositions() { return particlesPos; };

    /**
     * @brief Binds the position of the given particle between the simulation dimensions.
     *
     * @param particle A reference to the particle to be bound.
     */
    void bindPosition(Particle& particle);

    /**
     * @brief Binds the positions of all particles between the simulation dimensions.
     */
    void bindParticlesPositions();

    /**
     * @brief Loads the particle positions from a file.
     *
     * @param filePath The path of the file to load from.
     */
    void loadPositionsFromFile(const std::string& filePath);

    /**
     * @brief Checks if the particles have been initialized.
     *
     * @return True if the particles have been initialized, false otherwise.
     */
    bool areParticlesInitialized() { return isInitialized; }

private:
    int num;  // Number handled of particles
    std::vector<Particle> particles;
    std::vector<float> particlesPos;
    Physics& physics;

    size_t thread_count;
    ThreadPool pool;

    bool isInitialized;  // True if particles have been initialized
};

#endif //LAGRANGIAN_FLUID_SIMULATION_PARTICLES_HANDLER_H
