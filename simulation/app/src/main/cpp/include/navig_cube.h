//
// Created by martin on 14-06-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_NAVIG_CUBE_H
#define LAGRANGIAN_FLUID_SIMULATION_NAVIG_CUBE_H

#include <GLES3/gl32.h>
#include <glm/glm.hpp>

/**
 * @class NavigCube
 * @brief This class provides a navigational cube, i.e. the little coordinate system in the corner.
 */
class NavigCube {
public:

    /**
     * @brief Constructor
     */
    NavigCube();

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

    const GLfloat vertices[24] = {  // homogeneous coordinates
            // X axis
            0.0f, 0.0f, 0.0f, 1.0f, // origin
            -0.2f, 0.0f, 0.0f, 1.0f, // end of the x axis

            // Y axis
            0.0f, 0.0f, 0.0f, 1.0f, // origin
            0.0f, -0.2f, 0.0f, 1.0f, // end of the y axis

            // Z axis
            0.0f, 0.0f, 0.0f, 1.0f, // origin
            0.0f, 0.0f, 0.2f, 1.0f, // end of the z axis
    };

    const GLfloat colors[24] = {
            0.8f, 0.2402f, 0.7598f, 1.0f, // colour code for x axis
            0.8f, 0.2402f, 0.7598f, 1.0f, // colour code for x axis

            0.2f, 0.65f, 0.65f, 1.0f, // colour code for y axis
            0.2f, 0.65f, 0.65f, 1.0f, // colour code for y axis

            1.0f, 1.0f, 0.0f, 1.0f, // Yellow
            1.0f, 1.0f, 0.0f, 1.0f  // Yellow
    };

};

inline NavigCube::NavigCube() {
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

inline void NavigCube::draw(GLuint shaderProgram, glm::mat4& modelTransform) {
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelTransform"), 1, GL_TRUE, &modelTransform[0][0]);
    glDrawArrays(GL_LINES, 0, 6);
    glBindVertexArray(0);
}

inline void NavigCube::loadConstUniforms(GLuint shaderProgram) {
    glUseProgram(shaderProgram);
    glm::vec3 translate = glm::vec3(0.9f, -0.9f, 0.0f);
    glm::mat4x4 viewTransform = glm::translate(glm::identity<glm::mat4>(), translate);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "viewTransform"), 1, GL_FALSE, &viewTransform[0][0]);
}


#endif //LAGRANGIAN_FLUID_SIMULATION_NAVIG_CUBE_H
