//
// Created by martin on 08-05-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_TOUCH_HANDLER_H
#define LAGRANGIAN_FLUID_SIMULATION_TOUCH_HANDLER_H


#include "mainview.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

struct TouchPoint {
    float startX;
    float startY;
    float currentX;
    float currentY;
};


class TouchHandler {
public:
    TouchHandler(Transforms& transforms);

    void handleTouch(float x[2], float y[2], int action, int pointerCount);
    void handleSingleTouch(float x, float y, int action);
    void handleDoubleTouch(float x[2], float y[2], int action);

private:
    TouchPoint tpScale1;
    TouchPoint tpScale2;
    float prevScale;

    TouchPoint tpRot;
    glm::vec3 prevRot;

    Transforms& transform;
};

#endif //LAGRANGIAN_FLUID_SIMULATION_TOUCH_HANDLER_H
