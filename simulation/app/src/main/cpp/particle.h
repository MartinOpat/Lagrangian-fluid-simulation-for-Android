#ifndef PARTICLE_H
#define PARTICLE_H

// Include necessary libraries
#include "triple.h"
#include "vector_field_handler.h"

#include <vector>

// Class declaration
class Particle {
public:
    // Constructor
    Particle(Vec3 initialPosition, Vec3 initialVelocity);

    // Public methods
    void eulerStep(double dt, VectorFieldHandler& vectorFieldHandler); // Euler integration - mostly debug purposes
//    void eulerStep(double dt, const std::function<void(Point, Vec3&)>& velocityField); // Euler integration - mostly debug purposes
    void rk4Step(double dt, double b, VectorFieldHandler& vectorFieldHandler);
//    void rk4Step(double dt, const std::function<void(Point, Vec3&)>& velocityField, double b);
    Vec3 getPosition() const { return position; }
    void bindPosition();

private:
    // Private member variables
    Vec3 position;
    Vec3 velocity;

    // Private methods
};

#endif // PARTICLE_H