//
// Created by martin on 08-05-2024.
//

#ifndef PARTICLE_H
#define PARTICLE_H

// Include necessary libraries
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "consts.h"
#include "android_logging.h"
#include <vector>

/**
 * @class Particle
 * @brief This class represents a particle in a physics simulation.
 */
class Particle {
public:
    /**
     * @brief Constructor for the Particle class.
     *
     * @param initialPosition The initial position of the particle.
     * @param initialVelocity The initial velocity of the particle.
     * @param initialAcceleration The initial acceleration of the particle.
     */
    Particle(glm::vec3 initialPosition = glm::vec3(0.0f), glm::vec3 initialVelocity = glm::vec3(0.0f), glm::vec3 initialAcceleration = glm::vec3(0.0f));

    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
};

#endif // PARTICLE_H