#include "triple.h"
#include "android_logging.h"

#include <cmath>
#include <exception>
#include <iostream>

using namespace std;

// --- Constructors ------------------------------------------------------------

Triple::Triple(double X, double Y, double Z) : x(X), y(Y), z(Z) {}

// --- Operators ---------------------------------------------------------------

Triple Triple::operator+(Triple const &t) const { return Triple(x + t.x, y + t.y, z + t.z); }

Triple Triple::operator+(double f) const { return Triple(x + f, y + f, z + f); }

Triple Triple::operator-() const { return Triple(-x, -y, -z); }

Triple Triple::operator-(Triple const &t) const { return Triple(x - t.x, y - t.y, z - t.z); }

Triple Triple::operator-(double f) const { return Triple(x - f, y - f, z - f); }

Triple Triple::operator*(Triple const &t) const { return Triple(x * t.x, y * t.y, z * t.z); }

Triple Triple::operator*(double f) const { return Triple(x * f, y * f, z * f); }

Triple Triple::operator/(double f) const {
    double invf = 1.0 / f;
    return Triple(x * invf, y * invf, z * invf);
}

// --- Compound operators ------------------------------------------------------

Triple &Triple::operator+=(Triple const &t) {
    x += t.x;
    y += t.y;
    z += t.z;
    return *this;
}

Triple &Triple::operator+=(double f) {
    x += f;
    y += f;
    z += f;
    return *this;
}

Triple &Triple::operator-=(Triple const &t) {
    x -= t.x;
    y -= t.y;
    z -= t.z;
    return *this;
}

Triple &Triple::operator-=(double f) {
    x -= f;
    y -= f;
    z -= f;
    return *this;
}

Triple &Triple::operator*=(double f) {
    x *= f;
    y *= f;
    z *= f;
    return *this;
}

Triple &Triple::operator/=(double f) {
    double invf = 1.0 / f;
    x *= invf;
    y *= invf;
    z *= invf;
    return *this;
}

// --- Vector Operators --------------------------------------------------------

double Triple::dot(Triple const &t) const { return x * t.x + y * t.y + z * t.z; }

Triple Triple::cross(Triple const &t) const { return Triple(y * t.z - z * t.y, z * t.x - x * t.z, x * t.y - y * t.x); }

double Triple::length() const { return sqrt(length_2()); }

double Triple::length_2() const { return x * x + y * y + z * z; }

Triple Triple::normalized() const { return (*this) / length(); }

void Triple::normalize() {
    double len = length();
    double invlen = 1.0 / len;
    x *= invlen;
    y *= invlen;
    z *= invlen;
}

// --- Color functions ---------------------------------------------------------

void Triple::set(double f) {
    r = f;
    g = f;
    b = f;
}

void Triple::set(double f, double maxValue) { set(f / maxValue); }
void Triple::set(double red, double green, double blue) {
    r = red;
    g = green;
    b = blue;
}

void Triple::set(double red, double green, double blue, double maxValue) {
    set(red / maxValue, green / maxValue, blue / maxValue);
}

Triple &Triple::clamp(double maxValue) {
    r = fmin(r, maxValue);
    g = fmin(g, maxValue);
    b = fmin(b, maxValue);
    return *this;
}

// --- Free Operators ----------------------------------------------------------

Triple operator+(double f, Triple const &t) { return Triple(f + t.x, f + t.y, f + t.z); }

Triple operator-(double f, Triple const &t) { return Triple(f - t.x, f - t.y, f - t.z); }

Triple operator*(double f, Triple const &t) { return Triple(f * t.x, f * t.y, f * t.z); }

// --- IO Operators ------------------------------------------------------------

istream &operator>>(istream &is, Triple &t) {
    double x, y, z;
    //  is >> x >> y >> z;      // is not guaranteed to work pre C++17
    is >> x;
    is >> y;
    is >> z;
    t.set(x, y, z);  // only assign if everything is extracted
    return is;
}

ostream &operator<<(ostream &os, Triple const &t) {
    // format: [x, y, z] (no newline)
    os << '[' << t.x << ", " << t.y << ", " << t.z << ']';
    return os;
}

// --- Logging -----------------------------------------------------------------
void debugLogTriple(const char *msg, Triple const &t) {
    LOGI("(%f, %f, %f)\n", t.x, t.y, t.z);
}
