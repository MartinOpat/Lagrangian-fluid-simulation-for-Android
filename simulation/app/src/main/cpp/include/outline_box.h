//
// Created by martin on 03-07-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_OUTLINE_BOX_H
#define LAGRANGIAN_FLUID_SIMULATION_OUTLINE_BOX_H

#include <GLES3/gl32.h>
#include <glm/glm.hpp>

#include "consts.h"

class OutlineBox {
public:

    OutlineBox();

    /**
     * @brief Function to load the constant uniforms for the navigational cube.
     *
     * @param shaderProgram The shader program to load the uniforms to.
     */
    void loadConstUniforms(GLuint shaderProgram);

    /**
     * @brief Function to draw the navigational cube.
     *
     * @param shaderProgram The shader program to use.
     * @param modelTransform The current model transformation matrix.
     */
    void draw(GLuint shaderProgram, glm::mat4& modelTransform);

private:
    GLuint VBOs[2], VAO;

    const float zNegModif = 0.98f;
    const float yNegModif = 0.98f;
    const float xNegModif = 0.98f;
    const float zPosModif = 1.03f;
    const float yPosModif = 1.02f;
    const float xPosModif = 1.01f;
    const GLfloat vertices[96] = {
            // ___ Face
            -xPosModif*FIELD_WIDTH, -yPosModif*FIELD_HEIGHT, -zPosModif * FIELD_DEPTH, 1.0f,
            xNegModif * FIELD_WIDTH, -yPosModif*FIELD_HEIGHT, -zPosModif * FIELD_DEPTH, 1.0f,

            xNegModif * FIELD_WIDTH, -yPosModif*FIELD_HEIGHT, -zPosModif * FIELD_DEPTH, 1.0f,
            xNegModif * FIELD_WIDTH, yNegModif * FIELD_HEIGHT, -zPosModif * FIELD_DEPTH, 1.0f,

            xNegModif * FIELD_WIDTH, yNegModif * FIELD_HEIGHT, -zPosModif * FIELD_DEPTH, 1.0f,
            -xPosModif*FIELD_WIDTH, yNegModif * FIELD_HEIGHT, -zPosModif * FIELD_DEPTH, 1.0f,

            -xPosModif*FIELD_WIDTH, yNegModif * FIELD_HEIGHT, -zPosModif * FIELD_DEPTH, 1.0f,
            -xPosModif*FIELD_WIDTH, -yPosModif*FIELD_HEIGHT, -zPosModif * FIELD_DEPTH, 1.0f,

            -xPosModif*FIELD_WIDTH, -yPosModif*FIELD_HEIGHT, zNegModif * FIELD_DEPTH, 1.0f,
            xNegModif * FIELD_WIDTH, -yPosModif*FIELD_HEIGHT, zNegModif * FIELD_DEPTH, 1.0f,

            // ___ Face
            xNegModif * FIELD_WIDTH, -yPosModif*FIELD_HEIGHT, zNegModif * FIELD_DEPTH, 1.0f,
            xNegModif * FIELD_WIDTH, yNegModif * FIELD_HEIGHT, zNegModif * FIELD_DEPTH, 1.0f,

            xNegModif * FIELD_WIDTH, yNegModif * FIELD_HEIGHT, zNegModif * FIELD_DEPTH, 1.0f,
            -xPosModif*FIELD_WIDTH, yNegModif * FIELD_HEIGHT, zNegModif * FIELD_DEPTH, 1.0f,

            -xPosModif*FIELD_WIDTH, yNegModif * FIELD_HEIGHT, zNegModif * FIELD_DEPTH, 1.0f,
            -xPosModif*FIELD_WIDTH, -yPosModif*FIELD_HEIGHT, zNegModif * FIELD_DEPTH, 1.0f,

            // Connecting lines
            -xPosModif*FIELD_WIDTH, -yPosModif*FIELD_HEIGHT, -zPosModif * FIELD_DEPTH, 1.0f,
            -xPosModif*FIELD_WIDTH, -yPosModif*FIELD_HEIGHT, zNegModif * FIELD_DEPTH, 1.0f,

            xNegModif * FIELD_WIDTH, -yPosModif*FIELD_HEIGHT, -zPosModif * FIELD_DEPTH, 1.0f,
            xNegModif * FIELD_WIDTH, -yPosModif*FIELD_HEIGHT, zNegModif * FIELD_DEPTH, 1.0f,

            xNegModif * FIELD_WIDTH, yNegModif * FIELD_HEIGHT, -zPosModif * FIELD_DEPTH, 1.0f,
            xNegModif * FIELD_WIDTH, yNegModif * FIELD_HEIGHT, zNegModif * FIELD_DEPTH, 1.0f,

            -xPosModif*FIELD_WIDTH, yNegModif * FIELD_HEIGHT, -zPosModif * FIELD_DEPTH, 1.0f,
            -xPosModif*FIELD_WIDTH, yNegModif * FIELD_HEIGHT, zNegModif * FIELD_DEPTH, 1.0f,
    };

    const GLfloat colors[96] = {
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,

            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,

            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,

            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,

            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,

            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
    };

};

inline OutlineBox::OutlineBox() {
    glGenBuffers(2, VBOs);
    glGenVertexArrays(1, &VAO);

    // Bind VAO
    glBindVertexArray(VAO);

    // Load vertices
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Load colors
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Unbind VAO
    glBindVertexArray(0);
}

inline void OutlineBox::draw(GLuint shaderProgram, glm::mat4& modelTransform) {
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelTransform"), 1, GL_FALSE, &modelTransform[0][0]);
    glDrawArrays(GL_LINES, 0, 24);
    glBindVertexArray(0);
}

inline void OutlineBox::loadConstUniforms(GLuint shaderProgram) {
    glUseProgram(shaderProgram);
    glm::mat4x4 viewTransform = glm::identity<glm::mat4>();

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "viewTransform"), 1, GL_FALSE, &viewTransform[0][0]);
}


#endif //LAGRANGIAN_FLUID_SIMULATION_OUTLINE_BOX_H
