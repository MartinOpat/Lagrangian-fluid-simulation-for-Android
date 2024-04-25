// Adapted from the Computer Graphics Course at the University of Groningen


#ifndef TRIPLE_H_
#define TRIPLE_H_

#include <iosfwd>

// Color, Point and Vector are all Triples (name them so)
class Triple;
typedef Triple Color;
typedef Triple Point;
typedef Triple Vec3;

class Triple {
public:
    // --- data members ------------------------------------------------------------

    // union to acces the same elements by
    // x, y, z, or r, g, b or data[index]
    union {
        double data[3];
        struct {
            double x;
            double y;
            double z;
        };
        struct {
            double r;
            double g;
            double b;
        };
    };

    // --- Constructors ------------------------------------------------------------

    explicit Triple(double X = 0, double Y = 0, double Z = 0);

    // --- Operators ---------------------------------------------------------------

    Triple operator+(Triple const &t) const;  // add two triples
    Triple operator+(double f) const;         // add a value to each member
    // of a triple
    Triple operator-() const;                 // negate
    Triple operator-(Triple const &t) const;  // subtract two triples
    Triple operator-(double f) const;         // subtract a value from each
    // member

    Triple operator*(Triple const &t) const;  // memberwise multiplication
    Triple operator*(double f) const;         // multiply each member with a value
    Triple operator/(double f) const;         // divide each member by a value

    // --- Compound operators ------------------------------------------------------

    Triple &operator+=(Triple const &t);
    Triple &operator+=(double f);

    Triple &operator-=(Triple const &t);
    Triple &operator-=(double f);

    Triple &operator*=(double f);
    Triple &operator/=(double f);

    // --- Vector Operators --------------------------------------------------------

    double dot(Triple const &t) const;    // dot product
    Triple cross(Triple const &t) const;  // cross product

    double length() const;
    double length_2() const;  // length squared

    // NOTE: normalized returns a COPY, normalize does NOT
    Triple normalized() const;  // normalized COPY
    void normalize();           // normalize THIS

    // --- Color functions ---------------------------------------------------------

    void set(double f);                   // set all values to f
    void set(double f, double maxValue);  // set all values to f / maxVal
    void set(double red, double green, double blue);
    void set(double red, double green, double blue, double maxValue);

    Triple &clamp(double maxValue = 1.0);  // clamp: fmin(val, maxValue)
};

// --- Free Operators ----------------------------------------------------------

Triple operator+(double f, Triple const &t);
Triple operator-(double f, Triple const &t);
Triple operator*(double f, Triple const &t);

// --- IO Operators ------------------------------------------------------------

std::istream &operator>>(std::istream &is, Triple &t);
std::ostream &operator<<(std::ostream &os, Triple const &t);

// --- Logging -----------------------------------------------------------------

void debugLogTriple(const char *msg, Triple const &t);

#endif