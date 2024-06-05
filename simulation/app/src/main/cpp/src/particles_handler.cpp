//
// Created by martin on 08-05-2024.
//

#include "include/particles_handler.h"


ParticlesHandler::ParticlesHandler(InitType type, Physics& physics, int num) :
        physics(physics), num(num), pool(std::thread::hardware_concurrency()), thread_count(std::thread::hardware_concurrency()) {
    initParticles(type);
    isInitialized = true;
    srand(112358);  // Set rand() seed for reproducibility
}

ParticlesHandler::ParticlesHandler(Physics& physics, int num) :
        physics(physics), num(num), pool(std::thread::hardware_concurrency()), thread_count(std::thread::hardware_concurrency()) {
    isInitialized = false;
}


void ParticlesHandler::initParticles(InitType type) {
    particles.clear();
    particles.reserve(num);
    particlesPos.clear();
    particlesPos.reserve(num * 3);
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
        case InitType::uniform:
            for (int i = 0; i < num; i++) {
                // Randomly distribute particles uniformly over the 3D space

                float xPos = FIELD_WIDTH * (2 * (rand() / (float)RAND_MAX) - 1);
                float yPos = FIELD_HEIGHT * (2 * (rand() / (float)RAND_MAX) - 1);
                float zPos = FIELD_DEPTH * (2 * (rand() / (float)RAND_MAX) - 1);

                glm::vec3 initialPos(xPos, yPos, zPos);
                particles.push_back(Particle(initialPos, glm::vec3(0.0f, 0.0f, 0.0f)));
            }
            break;
    }

    // Populate particlesPos used for rendering
    particlesPos.clear();
    for (auto& particle : particles) {
        particlesPos.push_back(particle.position.x);
        particlesPos.push_back(particle.position.y);
        particlesPos.push_back(particle.position.z);
    }
}

inline void ParticlesHandler::bindPosition(Particle& particle) {
    particle.position.x = std::clamp(particle.position.x, -FIELD_WIDTH, FIELD_WIDTH);
    particle.position.y = std::clamp(particle.position.y, -FIELD_HEIGHT, FIELD_HEIGHT);
    particle.position.z = std::clamp(particle.position.z, -FIELD_DEPTH, FIELD_DEPTH);
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
    // setup threads
    int num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads(num_threads);
    auto chunk_size = particles.size() / num_threads;

    // worker function
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

    // Distribute work among threads
    size_t start_index = 0;
    auto begin = particles.begin();
    for (unsigned int i = 0; i < num_threads; i++) {
        auto end = (i == num_threads - 1) ? particles.end() : begin + chunk_size;
        threads[i] = std::thread(worker, begin, end, start_index);
        begin = end;
        start_index += chunk_size * 3;
    }

    // Join threads
    for (auto& thread : threads) {
        thread.join();
    }
}


void ParticlesHandler::updateParticlesPool() {
    size_t num_particles = particles.size();
    size_t batch_size = std::max(num_particles / thread_count, 1ul);  // Ensure non-zero batch size

    for (size_t i = 0; i < num_particles; i += batch_size) {
        pool.enqueue([this, i, batch_size, num_particles]() {
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

    // Make sure all jobs are done
    pool.waitForAll();
}

void ParticlesHandler::simulateParticles(Mainview& mainview) {
    if (mode == Mode::sequential) {
        updateParticles();
        mainview.loadParticlesData(particlesPos);
    } else if (mode == Mode::parallel) {
        updateParticlesPool();
        mainview.loadParticlesData(particlesPos);
    } else if (mode == Mode::computeShaders) {
        mainview.dispatchComputeShader();
    }
}

void ParticlesHandler::drawParticles(Mainview& mainview) {
    mainview.drawParticles(particlesPos.size());
}

void ParticlesHandler::bindParticlesPositions() {
    for (auto& particle : particles) {
        bindPosition(particle);
    }
}

void ParticlesHandler::loadPositionsFromFile(const std::string &filePath) {
    if (isInitialized) {
        LOGE("particles_handler", "Particles have already been initialized, ignoring file load.");
        std::remove(filePath.c_str());
        return;
    }
    netCDF::NcFile file(filePath, netCDF::NcFile::read);

    size_t numParticles = file.getDim("particle").getSize();

    // Read latitude, longitude, and depth
    std::vector<float> lats(numParticles), lons(numParticles), depths(numParticles);

    // Get the variables
    file.getVar("lat").getVar(lats.data());
    file.getVar("lon").getVar(lons.data());
    file.getVar("depth").getVar(depths.data());


    // Get the max values
    float maxLat, maxLon, maxDepth;
    file.getAtt("max_lat").getValues(&maxLat);
    file.getAtt("max_lon").getValues(&maxLon);
    file.getAtt("max_depth").getValues(&maxDepth);

    // Populate
    particlesPos.resize(numParticles * 3);
    for (size_t i = 0; i < numParticles; i++) {
        particlesPos[3 * i] = FIELD_WIDTH * ((lons[i] / maxLon) * 2 - 1);       // X (longitude)
        particlesPos[3 * i + 1] = FIELD_HEIGHT *  ((lats[i] / maxLat) * 2 - 1);   // Y (latitude)
        particlesPos[3 * i + 2] = FIELD_DEPTH *  ((depths[i] / maxDepth) * 2 - 1); // Z (depth)
    }

    // Cleanup
    file.close();
    std::remove(filePath.c_str());
}

