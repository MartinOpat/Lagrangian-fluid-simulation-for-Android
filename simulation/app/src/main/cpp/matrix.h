//
// Created by martin on 03-05-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_MATRIX_H
#define LAGRANGIAN_FLUID_SIMULATION_MATRIX_H


#include <iostream>

class Matrix4x4 {
public:
    // Elements stored in row-major order
    float data[16];

    // Constructors
    Matrix4x4();
    explicit Matrix4x4(float initVal);

    // Indexing operators
    float& operator()(int row, int col);
    const float& operator()(int row, int col) const;

    // Matrix operations
    Matrix4x4 operator+(const Matrix4x4& other) const;
    Matrix4x4 operator-(const Matrix4x4& other) const;
    Matrix4x4 operator*(const Matrix4x4& other) const;

    // Scalar multiplication
    Matrix4x4 operator*(float scalar) const;

    // Utility functions
    static Matrix4x4 translate(float dx, float dy, float dz);
    static Matrix4x4 scale(float sx, float sy, float sz);
    static Matrix4x4 rotateX(float angle);
    static Matrix4x4 rotateY(float angle);
    static Matrix4x4 rotateZ(float angle);
    static Matrix4x4 perspective(float fov, float aspect, float near, float far);
    void setToIdentity();

    // IO Operators
    friend std::ostream& operator<<(std::ostream& os, const Matrix4x4& m);
};

#endif //LAGRANGIAN_FLUID_SIMULATION_MATRIX_H
