#include "particle.h"

Particle::Particle(Vec3 initialPosition, Vec3 initialVelocity)
    : position(initialPosition), velocity(initialVelocity) {}


void Particle::eulerStep(double dt) {
    position += velocity * dt;
}

void Particle::rk4Step(double dt, const std::function<void(Point, Vec3&)>& velocityField, double b) {
    // Define k1, k2, k3, k4 for position and velocity
    Vec3 k1, k2, k3, k4;
    Vec3 v1, v2, v3, v4;

    auto computeVelocity = [&](Point pos, Vec3& vel) {
        Vec3 velField;
        velocityField(pos, velField); // Fluid velocity at particle's position
        vel = this->velocity - b * (this->velocity - velField);
    };

    // Initial adjusted velocity
    computeVelocity(this->position, v1);
    k1 = v1 * dt;

    // Update for k2
    computeVelocity(this->position + 0.5 * k1, v2);
    k2 = v2 * dt;

    // Update for k3
    computeVelocity(this->position + 0.5 * k2, v3);
    k3 = v3 * dt;

    // Update for k4
    computeVelocity(this->position + k3, v4);
    k4 = v4 * dt;

    // Update particle position
    this->position += (k1 + 2*k2 + 2*k3 + k4) / 6;

    // Update particle velocity
    this->velocity += (v1 + 2*v2 + 2*v3 + v4) / 6;
}
