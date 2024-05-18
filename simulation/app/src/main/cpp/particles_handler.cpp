//
// Created by martin on 08-05-2024.
//

#include "particles_handler.h"


ParticlesHandler::ParticlesHandler(InitType type, Physics& physics, int num) :
physics(physics), num(num), pool(std::thread::hardware_concurrency()), pool2(std::thread::hardware_concurrency()), thread_count(std::thread::hardware_concurrency()) {
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
    particlesPos.clear();
    for (auto& particle : particles) {
        particlesPos.push_back(particle.position.x);
        particlesPos.push_back(particle.position.y);
        particlesPos.push_back(particle.position.z);
    }
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
    int i = 0;
    for (auto& particle : particles) {
        physics.doStep(particle);
        bindPosition(particle);
        particlesPos[i++] = particle.position.x;
        particlesPos[i++] = particle.position.y;
        particlesPos[i++] = particle.position.z;
    }
}

void ParticlesHandler::updateParticlesParallel() {
    int num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads(num_threads);
    auto chunk_size = particles.size() / num_threads;

    auto worker = [this](auto begin, auto end, size_t start_index) {
        size_t i = start_index;
        for (auto it = begin; it != end; ++it) {
            physics.doStep(*it);
            bindPosition(*it);
            particlesPos[i++] = it->position.x;
            particlesPos[i++] = it->position.y;
            particlesPos[i++] = it->position.z;
        }
    };

    size_t start_index = 0;
    auto begin = particles.begin();
    for (unsigned int i = 0; i < num_threads; i++) {
        auto end = (i == num_threads - 1) ? particles.end() : begin + chunk_size;
        threads[i] = std::thread(worker, begin, end, start_index);
        begin = end;
        start_index += chunk_size * 3;
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

void ParticlesHandler::updateParticlesPool() {
    size_t num_particles = particles.size();
    size_t batch_size = std::max(num_particles / pool.get_thread_count(), 1ul);  // Ensure non-zero batch size

    for (size_t i = 0; i < num_particles; i += batch_size) {
        pool.submit_task([this, i, batch_size, num_particles]() {
            size_t end = std::min(i + batch_size, num_particles);
            for (size_t j = i; j < end; ++j) {
                physics.doStep(particles[j]);
                bindPosition(particles[j]);
                size_t index = j * 3;
                particlesPos[index] = particles[j].position.x;
                particlesPos[index + 1] = particles[j].position.y;
                particlesPos[index + 2] = particles[j].position.z;
            }
        });
    }
}

void ParticlesHandler::updateParticlesPool2() {
    size_t num_particles = particles.size();
    size_t batch_size = std::max(num_particles / thread_count, 1ul);  // Ensure non-zero batch size

    for (size_t i = 0; i < num_particles; i += batch_size) {
        pool2.enqueue([this, i, batch_size, num_particles]() {
            size_t end = std::min(i + batch_size, num_particles);
            for (size_t j = i; j < end; ++j) {
                physics.doStep(particles[j]);
                bindPosition(particles[j]);
                size_t index = j * 3;
                particlesPos[index] = particles[j].position.x;
                particlesPos[index + 1] = particles[j].position.y;
                particlesPos[index + 2] = particles[j].position.z;
            }
        });
    }
}

//void ParticlesHandler::updateParticlePositions() {
//    int i = 0;
//    for (auto& particle : particles) {
//        particlesPos[i++] = particle.position.x;
//        particlesPos[i++] = particle.position.y;
//        particlesPos[i++] = particle.position.z;
//    }
//}
//
//void ParticlesHandler::updateParticlePositionsParallel() {
//    unsigned int num_threads = std::thread::hardware_concurrency();
//    std::vector<std::thread> threads(num_threads);
//    auto chunk_size = particles.size() / num_threads;
//
//    auto worker = [this](auto begin, auto end, size_t start_index) {
//        size_t index = start_index;
//        for (auto it = begin; it != end; ++it) {
//            particlesPos[index++] = it->position.x;
//            particlesPos[index++] = it->position.y;
//            particlesPos[index++] = it->position.z;
//        }
//    };
//
//    size_t start_index = 0;
//    auto begin = particles.begin();
//    for (unsigned int i = 0; i < num_threads; ++i) {
//        auto end = (i == num_threads - 1) ? particles.end() : begin + chunk_size;
//        threads[i] = std::thread(worker, begin, end, start_index);
//        begin = end;
//        start_index += chunk_size * 3; // Move the start index by the number of floats processed
//    }
//
//    for (auto& thread : threads) {
//        thread.join();
//    }
//}
//
//void ParticlesHandler::updateParticlePositionsPool() {
//    size_t num_particles = particles.size();
//    size_t batch_size = std::max(num_particles / pool.get_thread_count(), 1ul);  // Calculate batch size based on thread count and ensure it's not zero
//
//    particlesPos.resize(num_particles * 3);  // Ensure the vector is appropriately sized
//
//    // Launch tasks in the thread pool
//    for (size_t i = 0; i < num_particles; i += batch_size) {
//        pool.submit_task([this, i, batch_size, num_particles]() {
//            size_t end = std::min(i + batch_size, num_particles);
//            for (size_t j = i; j < end; ++j) {
//                size_t index = j * 3;
//                particlesPos[index] = particles[j].position.x;
//                particlesPos[index + 1] = particles[j].position.y;
//                particlesPos[index + 2] = particles[j].position.z;
//            }
//        });
//    }
//}

void ParticlesHandler::drawParticles(Mainview& shaderManager) {
    updateParticles();
//    updateParticlesParallel();
//    updateParticlesPool();
//    updateParticlesPool2();
//    updateParticlePositions();
//    updateParticlePositionsParallel();
//    updateParticlePositionsPool();

    shaderManager.loadParticlesData(particlesPos);
    shaderManager.drawParticles(particlesPos.size());
}

void ParticlesHandler::bindParticlesPositions() {
    for (auto& particle : particles) {
        bindPosition(particle);
    }
}

