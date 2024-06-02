//
// Created by martin on 08-05-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_TOUCH_HANDLER_H
#define LAGRANGIAN_FLUID_SIMULATION_TOUCH_HANDLER_H


#include "mainview.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

/**
 * @struct TouchPoint
 * @brief This struct represents a point of touch in the application.
 */
struct TouchPoint {
    float startX;
    float startY;
    float currentX;
    float currentY;
};


/**
 * @class TouchHandler
 * @brief This class handles touch events in the application.
 */
class TouchHandler {
public:
    /**
     * @brief Constructor.
     *
     * @param transforms A reference to the transformations object.
     */
    TouchHandler(Transforms& transforms);

    /**
     * @brief Handles any touch event.
     *
     * @param x The x coordinates of the touch points.
     * @param y The y coordinates of the touch points.
     * @param action The action of the touch event.
     * @param pointerCount The number of pointers in the touch event.
     */
    void handleTouch(float x[2], float y[2], int action, int pointerCount);

    /**
     * @brief Handles a single-touch event.
     *
     * @param x The x coordinate of the touch point.
     * @param y The y coordinate of the touch point.
     * @param action The action of the touch event.
     */
    void handleSingleTouch(float x, float y, int action);

    /**
     * @brief Handles a double-touch event.
     *
     * @param x The x coordinates of the touch points.
     * @param y The y coordinates of the touch points.
     * @param action The action of the touch event.
     */
    void handleDoubleTouch(float x[2], float y[2], int action);

private:
    // Scaling
    TouchPoint tpScale1;
    TouchPoint tpScale2;
    float prevScale;

    // Rotation
    TouchPoint tpRot;
    glm::vec3 prevRot;
    Transforms& transform;
};

#endif //LAGRANGIAN_FLUID_SIMULATION_TOUCH_HANDLER_H
