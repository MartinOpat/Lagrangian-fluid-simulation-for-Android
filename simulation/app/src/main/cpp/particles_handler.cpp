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

void ParticlesHandler::updateParticles() {
    for (auto& particle : particles) {
        particle.rk4Step(physics);
        particle.bindPosition();
    }
}

void ParticlesHandler::updateParticlePositions() {
    particlesPos.clear();
    for (auto& particle : particles) {
        glm::vec3 particlePos = particle.getPosition();
        particlesPos.push_back(particlePos.x);
        particlesPos.push_back(particlePos.y);
        particlesPos.push_back(particlePos.z);
    }
}

void ParticlesHandler::drawParticles(GLShaderManager& shaderManager) {
    updateParticles();
    updateParticlePositions();
    shaderManager.loadParticlesData(particlesPos);
    shaderManager.drawParticles(particlesPos.size());
}

