#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <vector>
#include "framebuffer.h"
#include "polygon.h"

// A 2D point with sub-pixel precision (used before rounding to the pixel grid).
struct Vec2 {
    float x;
    float y;
};

// 3x3 matrix for 2D transforms in homogeneous coordinates.
// A point (x, y) is treated as the column vector [x, y, 1].
//
//   | a  b  tx |   | x |
//   | c  d  ty | * | y |
//   | 0  0  1  |   | 1 |
//
// Using 3x3 instead of 2x2 lets translation be a matrix too, so ANY
// sequence of translate / rotate / scale collapses into ONE matrix.
struct Mat3 {
    float m[3][3]; // m[row][col]

    static Mat3 identity();
    static Mat3 translate(float tx, float ty);
    static Mat3 rotate(float radians);       // positive = clockwise on screen (y points down)
    static Mat3 scale(float sx, float sy);

    // Matrix product. (A * B) applied to p == A applied to (B applied to p),
    // i.e. the RIGHT-most matrix happens FIRST.
    Mat3 operator*(const Mat3& other) const;

    Vec2 apply(Vec2 p) const;
};

// Rotate / scale around a pivot instead of the origin:
//   T(pivot) * M * T(-pivot)
// (move pivot to origin, transform, move back).
Mat3 aboutPoint(const Mat3& m, float pivotX, float pivotY);

// Applies M to every vertex and rounds to the pixel grid.
std::vector<Point> transformPoints(const Mat3& M, const std::vector<Vec2>& local);

// Transforms a shape defined in its own LOCAL coordinates (usually centred on
// (0,0)) into screen space and fills it with the scanline polygon filler.
void fillTransformedPolygon(Framebuffer& fb, const Mat3& M,
                            const std::vector<Vec2>& local, Color color);

// Convenience: a w x h rectangle centred on the local origin.
std::vector<Vec2> centredRect(float w, float h);

#endif
