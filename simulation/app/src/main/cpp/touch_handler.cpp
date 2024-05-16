//
// Created by martin on 08-05-2024.
//

#include "touch_handler.h"

bool isScaling = false;

TouchHandler::TouchHandler(Mainview& shaderManager) : shaderManager(shaderManager), prevRot(glm::vec3(0, 0, 0)), prevScale(0.5f) {
    tpScale1 = {0.0f, 0.0f, 0.0f, 0.0f};
    tpScale2 = {0.0f, 0.0f, 0.0f, 0.0f};
    tpRot = {0.0f, 0.0f, 0.0f, 0.0f};
}

void TouchHandler::handleTouch(float x[2], float y[2], int action, int pointerCount) {
    // 0, 5 -> click
    // 1, 6 -> release
    // 2 -> move
    if (pointerCount == 1) {
        handleSingleTouch(x[0], y[0], action);
    } else if (pointerCount == 2) {
        handleDoubleTouch(x, y, action);
    }
}

void TouchHandler::handleSingleTouch(float x, float y, int action) {
    if (isScaling) {
        isScaling = false;
        return;
    }

    if (action == 0) {
        tpRot.startX = x;
        tpRot.startY = y;
        tpRot.currentX = x;
        tpRot.currentY = y;
        prevRot = shaderManager.getRotation();
    } else if (action == 1) {
        tpRot.startX = 0.0f;
        tpRot.startY = 0.0f;
        tpRot.currentX = 0.0f;
        tpRot.currentY = 0.0f;
    } else if (action == 2) {
        tpRot.currentX = x;
        tpRot.currentY = y;

        float rotSensitivity = 0.001f;
        float dx = tpRot.currentX - tpRot.startX;
        float dy = tpRot.currentY - tpRot.startY;
        shaderManager.setRotation(rotSensitivity*dx + prevRot.x, rotSensitivity*dy + prevRot.y, prevRot.z);
    }
}

void TouchHandler::handleDoubleTouch(float x[2], float y[2], int action) {
    isScaling = true;
    if (action == 5) {
        tpScale1.startX = x[0];
        tpScale1.startY = y[0];
        tpScale1.currentX = x[0];
        tpScale1.currentY = y[0];
        tpScale2.startX = x[1];
        tpScale2.startY = y[1];
        tpScale2.currentX = x[1];
        tpScale2.currentY = y[1];
        prevScale = shaderManager.getScale();
    } else if (action == 6) {
        tpScale1.startX = 0.0f;
        tpScale1.startY = 0.0f;
        tpScale1.currentX = 0.0f;
        tpScale1.currentY = 0.0f;
        tpScale2.startX = 0.0f;
        tpScale2.startY = 0.0f;
        tpScale2.currentX = 0.0f;
        tpScale2.currentY = 0.0f;
    } else if (action == 2) {
        tpScale1.currentX = x[0];
        tpScale1.currentY = y[0];

        tpScale2.currentX = x[1];
        tpScale2.currentY = y[1];

        float currDist = (tpScale1.currentX - tpScale2.currentX)*(tpScale1.currentX - tpScale2.currentX) +
                         (tpScale1.currentY - tpScale2.currentY)*(tpScale1.currentY - tpScale2.currentY);
        float initDist = (tpScale1.startX - tpScale2.startX)*(tpScale1.startX - tpScale2.startX) +
                         (tpScale1.startY - tpScale2.startY)*(tpScale1.startY - tpScale2.startY);
        float scale = sqrt(currDist / initDist);
        shaderManager.setScale(scale * prevScale);
    }
}


