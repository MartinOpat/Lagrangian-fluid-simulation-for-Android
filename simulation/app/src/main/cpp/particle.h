#ifndef PARTICLE_H
#define PARTICLE_H

// Include necessary libraries
#include "triple.h"
#include <vector>

// Class declaration
class Particle {
public:
    // Constructor
    Particle(Vec3 initialPosition, Vec3 initialVelocity);

    // Public methods
    void updatePosition(double dt);

private:
    // Private member variables
    Vec3 position;
    Vec3 velocity;
};

#endif // PARTICLE_H