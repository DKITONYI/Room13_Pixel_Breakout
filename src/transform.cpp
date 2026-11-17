#include "transform.h"
#include <cmath>

Mat3 Mat3::identity() {
    return {{
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1},
    }};
}

Mat3 Mat3::translate(float tx, float ty) {
    return {{
        {1, 0, tx},
        {0, 1, ty},
        {0, 0, 1},
    }};
}

Mat3 Mat3::rotate(float radians) {
    const float c = std::cos(radians);
    const float s = std::sin(radians);
    return {{
        {c, -s, 0},
        {s,  c, 0},
        {0,  0, 1},
    }};
}

Mat3 Mat3::scale(float sx, float sy) {
    return {{
        {sx, 0,  0},
        {0,  sy, 0},
        {0,  0,  1},
    }};
}

Mat3 Mat3::operator*(const Mat3& o) const {
    Mat3 r{};
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            float sum = 0;
            for (int k = 0; k < 3; k++) {
                sum += m[row][k] * o.m[k][col];
            }
            r.m[row][col] = sum;
        }
    }
    return r;
}

Vec2 Mat3::apply(Vec2 p) const {
    // Bottom row is always (0, 0, 1) for these affine transforms,
    // so the homogeneous w stays 1 and needs no divide.
    return {
        m[0][0] * p.x + m[0][1] * p.y + m[0][2],
        m[1][0] * p.x + m[1][1] * p.y + m[1][2],
    };
}

Mat3 aboutPoint(const Mat3& m, float pivotX, float pivotY) {
    return Mat3::translate(pivotX, pivotY) * m * Mat3::translate(-pivotX, -pivotY);
}

std::vector<Point> transformPoints(const Mat3& M, const std::vector<Vec2>& local) {
    std::vector<Point> out;
    out.reserve(local.size());
    for (const Vec2& v : local) {
        Vec2 p = M.apply(v);
        out.push_back({static_cast<int>(std::lround(p.x)),
                       static_cast<int>(std::lround(p.y))});
    }
    return out;
}

void fillTransformedPolygon(Framebuffer& fb, const Mat3& M,
                            const std::vector<Vec2>& local, Color color) {
    fillPolygon(fb, transformPoints(M, local), color);
}

std::vector<Vec2> centredRect(float w, float h) {
    const float hw = w / 2, hh = h / 2;
    return {{-hw, -hh}, {hw, -hh}, {hw, hh}, {-hw, hh}};
}
