//
// Created by martin on 03-05-2024.
//

#ifndef LAGRANGIAN_FLUID_SIMULATION_MATRIX_H
#define LAGRANGIAN_FLUID_SIMULATION_MATRIX_H


#include <iostream>

class Matrix4x4 {
public:
    // Elements stored in row-major order
    double data[16];

    // Constructors
    Matrix4x4();
    explicit Matrix4x4(double initVal);

    // Indexing operators
    double& operator()(int row, int col);
    const double& operator()(int row, int col) const;

    // Matrix operations
    Matrix4x4 operator+(const Matrix4x4& other) const;
    Matrix4x4 operator-(const Matrix4x4& other) const;
    Matrix4x4 operator*(const Matrix4x4& other) const;

    // Scalar multiplication
    Matrix4x4 operator*(double scalar) const;

    // Utility functions
    static Matrix4x4 translate(double dx, double dy, double dz);
    static Matrix4x4 scale(double sx, double sy, double sz);
    static Matrix4x4 rotateX(double angle);
    static Matrix4x4 rotateY(double angle);
    static Matrix4x4 rotateZ(double angle);

    // IO Operators
    friend std::ostream& operator<<(std::ostream& os, const Matrix4x4& m);
};

#endif //LAGRANGIAN_FLUID_SIMULATION_MATRIX_H
