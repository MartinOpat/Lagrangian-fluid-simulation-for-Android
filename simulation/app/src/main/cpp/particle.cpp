#include "particle.h"
#include "android_logging.h"

Particle::Particle(glm::vec3 initialPosition, glm::vec3 initialVelocity, glm::vec3 initialAcceleration)
    : position(initialPosition), velocity(initialVelocity), acceleration(initialAcceleration)  {}


void Particle::eulerStep(Physics& physics) {
    this->acceleration = physics.dvdt(this->position, this->velocity);
    this->velocity += this->acceleration * physics.dt;
    this->position += this->velocity * physics.dt;
}

void Particle::bindPosition() {
    if (this->position.x < -FIELD_WIDTH) {
        this->position.x = -FIELD_WIDTH;
        this->velocity.x = 0;
    } else if (this->position.x > FIELD_WIDTH) {
        this->position.x = FIELD_WIDTH;
        this->velocity.x = 0;
    }

    if (this->position.y < -FIELD_HEIGHT) {
        this->position.y = -FIELD_HEIGHT;
        this->velocity.y = 0;
    } else if (this->position.y > FIELD_HEIGHT) {
        this->position.y = FIELD_HEIGHT;
        this->velocity.y = 0;
    }

    if (this->position.z < -FIELD_DEPTH) {
        this->position.z = -FIELD_DEPTH;
        this->velocity.z = 0;
    } else if (this->position.z > FIELD_DEPTH) {
        this->position.z = FIELD_DEPTH;
        this->velocity.z = 0;
    }
}

void Particle::rk4Step(Physics& physics) {
    // Define k1, k2, k3, k4 for position and velocity
    glm::vec3 a1, a2, a3, a4;
    glm::vec3 v1, v2, v3, v4;

    float dt = physics.dt;

    // Initial adjusted velocity
    a1 = physics.dvdt({this->position, this->velocity, this->acceleration});
    v1 = this->velocity;

    // Update for k2
    a2 = physics.dvdt({this->position + 0.5f * v1 * dt, this->velocity + 0.5f * a1 * dt, this->acceleration});
    v2 = this->velocity + 0.5f * a1 * dt;

    // Update for k3
    a3 = physics.dvdt({this->position + 0.5f * v2 * dt, this->velocity + 0.5f * a2 * dt, this->acceleration});
    v3 = this->velocity + 0.5f * a2 * dt;

    // Update for k4
    a4 = physics.dvdt({this->position + v3 * dt, this->velocity + a3 * dt, this->acceleration});
    v4 = this->velocity + a3 * dt;

    this->acceleration = (a1 + 2.0f * a2 + 2.0f * a3 + a4) / 6.0f;
    this->velocity += dt * (a1 + 2.0f * a2 + 2.0f * a3 + a4) / 6.0f;
    this->position += dt * (v1 + 2.0f * v2 + 2.0f * v3 + v4) / 6.0f;
}
