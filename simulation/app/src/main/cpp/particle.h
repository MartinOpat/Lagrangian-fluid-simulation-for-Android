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
    void eulerStep(double dt, const std::function<void(Point, Vec3&)>& velocityField); // Euler integration - mostly debug purposes
    void rk4Step(double dt, const std::function<void(Point, Vec3&)>& velocityField, double b);
    Vec3 getPosition() const { return position; }

private:
    // Private member variables
    Vec3 position;
    Vec3 velocity;

    // Private methods

};

#endif // PARTICLE_H