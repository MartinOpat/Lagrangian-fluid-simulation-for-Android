//
// Created by martin on 08-05-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_VECTOR_FIELD_HANDLER_H
#define LAGRANGIAN_FLUID_SIMULATION_VECTOR_FIELD_HANDLER_H

#include "mainview.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <vector>

class VectorFieldHandler {
public:
    VectorFieldHandler(int finenessXY = 15, int finenessZ = 5);
    void velocityField(const glm::vec3 &position, glm::vec3 &velocity);

    void prepareVertexData(const std::vector<float>& uData, const std::vector<float>& vData);
    void prepareVertexData(const std::vector<float>& uData, const std::vector<float>& vData, const std::vector<float>& wData);

    void loadTimeStep(const std::string& fileUPath, const std::string& fileVPath);
    void loadTimeStepHelper(const std::string& fileUPath, const std::string& fileVPath, const std::string& fileWPath);
    void loadTimeStep(int fdU, int fdV, int fdW);

    void updateTimeStep();

    void draw(Mainview& shaderManager);

    std::vector<float>& getOldVertices() {return allVertices[0];};
    std::vector<float>& getNewVertices() {return allVertices[1];};
    std::vector<float>& getFutureVertices() {return allVertices[2];};

    int getWidth() {return width;};
    int getHeight() {return height;};
    int getDepth() {return depth;};

private:
    int width = 0 ;
    int height = 0;
    int depth = 0;
    int finenessXY;
    int finenessZ;
    std::vector<std::vector<float>> allVertices;
    std::vector<std::vector<float>> displayVertices;

};

#endif //LAGRANGIAN_FLUID_SIMULATION_VECTOR_FIELD_HANDLER_H