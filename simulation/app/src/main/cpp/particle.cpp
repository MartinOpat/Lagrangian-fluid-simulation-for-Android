#include "particle.h"
#include "android_logging.h"

Particle::Particle(Vec3 initialPosition, Vec3 initialVelocity)
    : position(initialPosition), velocity(initialVelocity) {}


void Particle::eulerStep(double dt, VectorFieldHandler& vectorFieldHandler) {
    vectorFieldHandler.velocityField(this->position, this->velocity);
    this->position += this->velocity * dt;
}

void Particle::bindPosition() {
    if (this->position.x < -1) {
        this->position.x = -1;
        this->velocity.x = 0;
    } else if (this->position.x > 1) {
        this->position.x = 1;
        this->velocity.x = 0;
    }

    if (this->position.y < -1) {
        this->position.y = -1;
        this->velocity.y = 0;
    } else if (this->position.y > 1) {
        this->position.y = 1;
        this->velocity.y = 0;
    }

    if (this->position.z < -1) {
        this->position.z = -1;
        this->velocity.z = 0;
    } else if (this->position.z > 1) {
        this->position.z = 1;
        this->velocity.z = 0;
    }
}

void Particle::rk4Step(double dt, double b, VectorFieldHandler& vectorFieldHandler) {
    // Define k1, k2, k3, k4 for position and velocity
    Vec3 a1, a2, a3, a4;
    Vec3 v1, v2, v3, v4;

    auto dvdt = [&](Point pos, Vec3 vel) {
        Vec3 velField;
        vectorFieldHandler.velocityField(pos, velField); // Fluid velocity at particle's position
        return - b * (vel - velField);
    };

    // Initial adjusted velocity
    a1 = dvdt(this->position, this->velocity);
    v1 = this->velocity;

    // Update for k2
    a2 = dvdt(this->position + 0.5 * v1 * dt, this->velocity + 0.5 * a1 * dt);
    v2 = this->velocity + 0.5 * v1 * dt;

    // Update for k3
    a3 = dvdt(this->position + 0.5 * v2 * dt, this->velocity + 0.5 * a2 * dt);
    v3 = this->velocity + 0.5 * v2 * dt;

    // Update for k4
    a4 = dvdt(this->position + v3 * dt, this->velocity + a3 * dt);
    v4 = this->velocity + v3 * dt;

    this->velocity += dt * (a1 + 2 * a2 + 2 * a3 + a4) / 6;
    this->position += dt * (v1 + 2 * v2 + 2 * v3 + v4) / 6;
}
