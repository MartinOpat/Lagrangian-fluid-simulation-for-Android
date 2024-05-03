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

Matrix4x4::Matrix4x4(double initVal) {
    for (int i = 0; i < 16; ++i) {
        data[i] = initVal;
    }
}

// Indexing operators
double& Matrix4x4::operator()(int row, int col) {
    return data[row * 4 + col];
}

const double& Matrix4x4::operator()(int row, int col) const {
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

Matrix4x4 Matrix4x4::operator*(double scalar) const {
    Matrix4x4 result;
    for (int i = 0; i < 16; ++i) {
        result.data[i] = this->data[i] * scalar;
    }
    return result;
}

// Utility functions
Matrix4x4 Matrix4x4::translate(double dx, double dy, double dz) {
    Matrix4x4 result;
    result(0, 3) = dx;
    result(1, 3) = dy;
    result(2, 3) = dz;
    return result;
}

Matrix4x4 Matrix4x4::scale(double sx, double sy, double sz) {
    Matrix4x4 result;
    result(0, 0) = sx;
    result(1, 1) = sy;
    result(2, 2) = sz;
    return result;
}

Matrix4x4 Matrix4x4::rotateX(double angle) {
    Matrix4x4 result;
    double c = cos(angle);
    double s = sin(angle);
    result(1, 1) = c;
    result(1, 2) = -s;
    result(2, 1) = s;
    result(2, 2) = c;
    return result;
}

Matrix4x4 Matrix4x4::rotateY(double angle) {
    Matrix4x4 result;
    double c = cos(angle);
    double s = sin(angle);
    result(0, 0) = c;
    result(0, 2) = s;
    result(2, 0) = -s;
    result(2, 2) = c;
    return result;
}

Matrix4x4 Matrix4x4::rotateZ(double angle) {
    Matrix4x4 result;
    double c = cos(angle);
    double s = sin(angle);
    result(0, 0) = c;
    result(0, 1) = -s;
    result(1, 0) = s;
    result(1, 1) = c;
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
