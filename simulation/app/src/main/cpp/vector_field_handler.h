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
    VectorFieldHandler(int finenessXY = 15, int finenessZ = 3);
    void velocityField(const glm::vec3 &position, glm::vec3 &velocity);

    void prepareVertexData(const std::vector<float>& uData, const std::vector<float>& vData);
    void prepareVertexData(const std::vector<float>& uData, const std::vector<float>& vData, const std::vector<float>& wData);

    void loadAllTimeSteps(const std::string& fileUPath, const std::string& fileVPath);
    void loadAllTimeSteps(const std::string& fileUPath, const std::string& fileVPath, const std::string& fileWPath);

    void draw(GLShaderManager& shaderManager);

    std::vector<float> getAllVertices() {return allVertices[currentFrame];};

private:
    int width = 0 ;
    int height = 0;
    int depth = 0;
    int currentFrame = 0;
    int numVertices = 0;
    int finenessXY = 1;
    int finenessZ = 1;
    std::vector<float> vertices;
    std::vector<std::vector<float>> allVertices;
    std::vector<std::vector<float>> displayVertices;

};

#endif //LAGRANGIAN_FLUID_SIMULATION_VECTOR_FIELD_HANDLER_H
