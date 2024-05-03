//
// Created by martin on 03-05-2024.
//

#include "matrix.h"
#include <iomanip>
#include <cmath>

// Constructors
Matrix4x4::Matrix4x4() {
    for (int i = 0; i < 16; ++i) {
        data[i] = (i % 5 == 0) ? 1.0 : 0.0;  // Identity matrix
    }
}

Matrix4x4::Matrix4x4(float initVal) {
    for (int i = 0; i < 16; ++i) {
        data[i] = initVal;
    }
}

// Indexing operators
float& Matrix4x4::operator()(int row, int col) {
    return data[row * 4 + col];
}

const float& Matrix4x4::operator()(int row, int col) const {
    return data[row * 4 + col];
}

// Matrix operations
Matrix4x4 Matrix4x4::operator+(const Matrix4x4& other) const {
    Matrix4x4 result;
    for (int i = 0; i < 16; ++i) {
        result.data[i] = this->data[i] + other.data[i];
    }
    return result;
}

Matrix4x4 Matrix4x4::operator-(const Matrix4x4& other) const {
    Matrix4x4 result;
    for (int i = 0; i < 16; ++i) {
        result.data[i] = this->data[i] - other.data[i];
    }
    return result;
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const {
    Matrix4x4 result(0.0);
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            for (int k = 0; k < 4; ++k) {
                result(row, col) += (*this)(row, k) * other(k, col);
            }
        }
    }
    return result;
}

Matrix4x4 Matrix4x4::operator*(float scalar) const {
    Matrix4x4 result;
    for (int i = 0; i < 16; ++i) {
        result.data[i] = this->data[i] * scalar;
    }
    return result;
}

// Utility functions
Matrix4x4 Matrix4x4::translate(float dx, float dy, float dz) {
    Matrix4x4 result;
    result(0, 3) = dx;
    result(1, 3) = dy;
    result(2, 3) = dz;
    return result;
}

Matrix4x4 Matrix4x4::scale(float sx, float sy, float sz) {
    Matrix4x4 result;
    result(0, 0) = sx;
    result(1, 1) = sy;
    result(2, 2) = sz;
    return result;
}

Matrix4x4 Matrix4x4::rotateX(float angle) {
    Matrix4x4 result;
    float c = cos(angle);
    float s = sin(angle);
    result(1, 1) = c;
    result(1, 2) = -s;
    result(2, 1) = s;
    result(2, 2) = c;
    return result;
}

Matrix4x4 Matrix4x4::rotateY(float angle) {
    Matrix4x4 result;
    float c = cos(angle);
    float s = sin(angle);
    result(0, 0) = c;
    result(0, 2) = s;
    result(2, 0) = -s;
    result(2, 2) = c;
    return result;
}

Matrix4x4 Matrix4x4::rotateZ(float angle) {
    Matrix4x4 result;
    float c = cos(angle);
    float s = sin(angle);
    result(0, 0) = c;
    result(0, 1) = -s;
    result(1, 0) = s;
    result(1, 1) = c;
    return result;
}

void Matrix4x4::setToIdentity() {
    for (int i = 0; i < 16; ++i) {
        data[i] = (i % 5 == 0) ? 1.0 : 0.0; // Identity matrix elements
    }
}

Matrix4x4 Matrix4x4::perspective(float fov, float aspect, float near, float far) {
    float const tanHalfFOV = tan(fov / 2.0);
    Matrix4x4 result(0.0);  // Initialize all elements to 0

    result(0, 0) = 1.0 / (aspect * tanHalfFOV);
    result(1, 1) = 1.0 / (tanHalfFOV);
    result(2, 2) = -(far + near) / (far - near);
    result(2, 3) = -1.0;
    result(3, 2) = -(2.0 * far * near) / (far - near);

    return result;
}


// IO Operators
std::ostream& operator<<(std::ostream& os, const Matrix4x4& m) {
    os << std::fixed << std::setprecision(2);
    for (int i = 0; i < 4; i++) {
        os << "[";
        for (int j = 0; j < 4; j++) {
            os << std::setw(6) << m(i, j) << (j < 3 ? ", " : "");
        }
        os << "]" << (i < 3 ? "\n" : "");
    }
    return os;
}
