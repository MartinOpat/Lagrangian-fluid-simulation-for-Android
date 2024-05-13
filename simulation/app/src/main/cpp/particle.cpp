#include "particle.h"
#include "android_logging.h"

Particle::Particle(glm::vec3 initialPosition, glm::vec3 initialVelocity, glm::vec3 initialAcceleration)
    : position(initialPosition), velocity(initialVelocity), acceleration(initialAcceleration)  {}



