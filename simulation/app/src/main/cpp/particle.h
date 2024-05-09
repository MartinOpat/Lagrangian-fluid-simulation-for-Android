#ifndef PARTICLE_H
#define PARTICLE_H

// Include necessary libraries
#include "triple.h"
#include "vector_field_handler.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <vector>

// Class declaration
class Particle {
public:
    // Constructor
    Particle(glm::vec3 initialPosition, glm::vec3 initialVelocity);

    // Public methods
    void eulerStep(float dt, VectorFieldHandler& vectorFieldHandler); // Euler integration - mostly debug purposes
    void rk4Step(float dt, float b, VectorFieldHandler& vectorFieldHandler);
    glm::vec3 getPosition() const { return position; }
    void bindPosition();

private:
    // Private member variables
    glm::vec3 position;
    glm::vec3 velocity;

    // Private methods
};

#endif // PARTICLE_H