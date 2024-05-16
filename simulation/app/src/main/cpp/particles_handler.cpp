//
// Created by martin on 08-05-2024.
//

#include "particles_handler.h"

ParticlesHandler::ParticlesHandler(InitType type, Physics& physics, int num) : physics(physics), num(num) {
    initParticles(type);
}

void ParticlesHandler::initParticles(InitType type) {
    particles.clear();
    particlesPos.clear();
    switch (type) {
        case InitType::line:
            for (int i = 0; i < num; i++) {
                // Zero initial velocity, diagonal initial position
                glm::vec3 initialVel(0.0f, 0.0f, 0.0f);
                float xPos = FIELD_WIDTH * (2 * (i / (float) num) - 1);
                float yPos = FIELD_HEIGHT * (2 * (i / (float) num) - 1);
                float zPos = FIELD_DEPTH * (2 * (i / (float) num) - 1);
                glm::vec3 initialPos(xPos, yPos, zPos);
                particles.push_back(Particle(initialPos, initialVel));
            }
            break;
        case InitType::two_lines:
            for (int i = 0; i < num; i++) {
                // Zero initial velocity, half-diagonal position
                glm::vec3 initialVel(0.0f, 0.0f, 0.0f);
                float xPos = FIELD_WIDTH * (i % 2 ? (i / (float) num) - 1 : 1 - (i / (float) num));
                float yPos = FIELD_HEIGHT * (2 * (i / (float) num) - 1);
                float zPos = FIELD_DEPTH * (2 * (i / (float) num) - 1);
                glm::vec3 initialPos(xPos, yPos, zPos);
                particles.push_back(Particle(initialPos, initialVel));
            }
            break;
        case InitType::explosion:
            for (int i = 0; i < num; i++) {
                // Randomly generate initial velocity
                float aspectRatio = 19.3f / 9.0f;
                float angle = 2.0f * M_PI * rand() / (float)RAND_MAX;
                float magnitude = 0.6f * rand() / (float)RAND_MAX;
                float xVel = FIELD_WIDTH * (magnitude * cos(angle) / aspectRatio);
                float yVel = FIELD_HEIGHT * (magnitude * sin(angle));
                float zVel = FIELD_DEPTH * (2 * (i / (float) num) - 1);
                glm::vec3 initialVel(xVel, yVel, zVel);
                glm::vec3 initialPos(-0.25f, 0.25f, 0.0f);
                particles.push_back(Particle(initialPos, initialVel));
            }
            break;
    }
    updateParticlePositions();
}

void ParticlesHandler::bindPosition(Particle& particle) {
    if (particle.position.x < -FIELD_WIDTH) {
        particle.position.x = -FIELD_WIDTH;
        particle.velocity.x = 0;
    } else if (particle.position.x > FIELD_WIDTH) {
        particle.position.x = FIELD_WIDTH;
        particle.velocity.x = 0;
    }

    if (particle.position.y < -FIELD_HEIGHT) {
        particle.position.y = -FIELD_HEIGHT;
        particle.velocity.y = 0;
    } else if (particle.position.y > FIELD_HEIGHT) {
        particle.position.y = FIELD_HEIGHT;
        particle.velocity.y = 0;
    }

    if (particle.position.z < -FIELD_DEPTH) {
        particle.position.z = -FIELD_DEPTH;
        particle.velocity.z = 0;
    } else if (particle.position.z > FIELD_DEPTH) {
        particle.position.z = FIELD_DEPTH;
        particle.velocity.z = 0;
    }
}

void ParticlesHandler::updateParticles() {
    for (auto& particle : particles) {
        physics.doStep(particle);
        bindPosition(particle);
    }
}

void ParticlesHandler::updateParticlePositions() {
    particlesPos.clear();
    for (auto& particle : particles) {
        particlesPos.push_back(particle.position.x);
        particlesPos.push_back(particle.position.y);
        particlesPos.push_back(particle.position.z);
    }
}

void ParticlesHandler::drawParticles(Mainview& shaderManager) {
    updateParticles();
    updateParticlePositions();
    shaderManager.loadParticlesData(particlesPos);
    shaderManager.drawParticles(particlesPos.size());
}

void ParticlesHandler::bindParticlesPositions() {
    for (auto& particle : particles) {
        bindPosition(particle);
    }
}

