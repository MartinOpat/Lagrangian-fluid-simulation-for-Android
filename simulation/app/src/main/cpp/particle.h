#ifndef PARTICLE_H
#define PARTICLE_H

// Include necessary libraries
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "consts.h"

#include <vector>

// Class declaration
class Particle {
public:
    // Constructor
    Particle(glm::vec3 initialPosition = glm::vec3(0.0f), glm::vec3 initialVelocity = glm::vec3(0.0f), glm::vec3 initialAcceleration = glm::vec3(0.0f));

    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
private:

};

#endif // PARTICLE_H