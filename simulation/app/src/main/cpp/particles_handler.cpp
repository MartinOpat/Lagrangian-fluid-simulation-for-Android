//
// Created by martin on 08-05-2024.
//

#include "particles_handler.h"

ParticlesHandler::ParticlesHandler(InitType type, VectorFieldHandler& vectorFieldHandler, int num, float dt, float b) : vectorFieldHandler(vectorFieldHandler), num(num), dt(dt), b(b) {
    initParticles(type);
}

void ParticlesHandler::initParticles(InitType type) {
    particles.clear();
    particlesPos.clear();
    switch (type) {
        case InitType::line:
            for (int i = 0; i < num; i++) {
                // Zero initial velocity, diagonal initial position
                Vec3 initialVel(0.0f, 0.0f, 0.0f);
                float xPos = 2 * (i / (float) num) - 1;
                float yPos = 2 * (i / (float) num) - 1;
                float zPos = 0.0f;
                Vec3 initialPos(xPos, yPos, zPos);
                particles.push_back(Particle(initialPos, initialVel));
            }
            break;
        case InitType::two_lines:
            for (int i = 0; i < num; i++) {
                // Zero initial velocity, half-diagonal position
                Vec3 initialVel(0.0f, 0.0f, 0.0f);
                float xPos = 2 * (i / (float) num) - 1;
                float yPos = i % 2 ? (i / (float) num) - 1 : 1 - (i / (float) num);
                float zPos = 0.0f;
                Vec3 initialPos(xPos, yPos, zPos);
                particles.push_back(Particle(initialPos, initialVel));
            }
            break;
        case InitType::explosion:
            for (int i = 0; i < num; i++) {
                // Randomly generate initial velocity
                float aspectRatio = 19.3f / 9.0f;
                float angle = 2.0f * M_PI * rand() / (float)RAND_MAX;
                float magnitude = 0.3f * rand() / (float)RAND_MAX;
                float xVel = magnitude * cos(angle) * aspectRatio;
                float yVel = magnitude * sin(angle);
                float zVel = 0.0f;
                Vec3 initialVel(xVel, yVel, zVel);
                Vec3 initialPos(-0.25f, 0.25f, 0.0f);
                particles.push_back(Particle(initialPos, initialVel));
            }
            break;
    }
    updateParticlePositions();
}

void ParticlesHandler::updateParticles() {
    for (auto& particle : particles) {
        particle.rk4Step(dt, b, vectorFieldHandler);
        particle.bindPosition();
    }
}

void ParticlesHandler::updateParticlePositions() {
    particlesPos.clear();
    for (auto& particle : particles) {
        Vec3 particlePos = particle.getPosition();
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

