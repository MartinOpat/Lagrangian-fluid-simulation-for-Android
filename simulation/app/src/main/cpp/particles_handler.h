//
// Created by martin on 08-05-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_PARTICLES_HANDLER_H
#define LAGRANGIAN_FLUID_SIMULATION_PARTICLES_HANDLER_H

#include "particle.h"
#include "mainview.h"

#include <stdio.h>
#include <vector>

class ParticlesHandler {
public:
    enum class InitType {
        line,       // A single diagonal line
        two_lines,      // Two diagonal lines
        explosion   // Explosion of particles
    };

    ParticlesHandler(InitType type, const std::function<void(Point, Vec3&)>& velocityField, int num = 100, float dt = 0.02f, float b = 0.8f);
    void initParticles(InitType type);
    void updateParticles();
    void updateParticlePositions();
    void drawParticles(GLShaderManager& shaderManager);
    std::vector<float> getParticlesPositions() { return particlesPos; };

private:
    int num = 100;  // Number of particles
    float dt = 0.02f;  // Time step
    float b = 0.8f;  // Drag coefficient
    std::vector<Particle> particles;
    std::vector<float> particlesPos;
    const std::function<void(Point, Vec3&)>& velocityField;

};

#endif //LAGRANGIAN_FLUID_SIMULATION_PARTICLES_HANDLER_H
