#include "particle.h"

Particle::Particle(Vec3 initialPosition, Vec3 initialVelocity)
    : position(initialPosition), velocity(initialVelocity) {}


void Particle::updatePosition(double dt) {
    position += velocity * dt;
}