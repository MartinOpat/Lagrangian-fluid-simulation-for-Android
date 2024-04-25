#ifndef PARTICLE_H
#define PARTICLE_H

// Include necessary libraries
#include <vector>

// Class declaration
class Particle {
public:
    // Constructor
    Particle();

    // Destructor
    ~Particle();

    // Public methods
    void update();

private:
    // Private member variables
    float position;
    float velocity;

    // Private methods
    void calculateNewPosition();
};

#endif // PARTICLE_H