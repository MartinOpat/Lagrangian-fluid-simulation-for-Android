//
// Created by martin on 08-05-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_PARTICLES_HANDLER_H
#define LAGRANGIAN_FLUID_SIMULATION_PARTICLES_HANDLER_H

#include "particle.h"
#include "mainview.h"
#include "physics.h"
#include "vector_field_handler.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <stdio.h>
#include <vector>
#include <algorithm>
#include <thread>

class ParticlesHandler {
public:
    enum class InitType {
        line,       // A single diagonal line
        two_lines,      // Two diagonal lines
        explosion   // Explosion of particles
    };

    ParticlesHandler(InitType type, Physics& physics, int num = 100);
    void initParticles(InitType type);

    void updateParticles();
    void updateParticlesParallel();
    void updateParticlePositions();
    void updateParticlePositionsParallel();

    void drawParticles(Mainview& shaderManager);
    std::vector<float> getParticlesPositions() { return particlesPos; };

    void bindPosition(Particle& particle);
    void bindParticlesPositions();

private:
    int num;  // Number of particles
    std::vector<Particle> particles;
    std::vector<float> particlesPos;
    Physics& physics;

};

#endif //LAGRANGIAN_FLUID_SIMULATION_PARTICLES_HANDLER_H
