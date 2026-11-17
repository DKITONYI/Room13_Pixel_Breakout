#include "breakout.h"
#include "polygon.h"
#include "circle.h"
#include "transform.h"
#include <cmath>

namespace {

// Layout constants (pixels).
const int BRICK_W   = 68;
const int BRICK_H   = 22;
const int BRICK_GAP = 6;
const int BRICK_TOP = 60;   // distance from top of screen to first row

const int PADDLE_W      = 120;
const int PADDLE_H      = 16;
const int PADDLE_BOTTOM = 40;  // gap between paddle and bottom of screen
const float PADDLE_BEVEL = 6;  // size of the cut-off corners

const int BALL_RADIUS = 14;
const int BALL_GAP    = 4;     // space between ball and paddle at the start

// One colour per row, top to bottom; wraps if there are more rows.
const Color ROW_COLORS[] = {
    {230,  57,  70},  // red
    {244, 132,  52},  // orange
    {244, 196,  48},  // yellow
    { 87, 190,  98},  // green
    { 66, 153, 225},  // blue
    {155,  93, 229},  // purple
};
const int NUM_ROW_COLORS = sizeof(ROW_COLORS) / sizeof(ROW_COLORS[0]);

Color darken(Color c, float k) {
    return {static_cast<unsigned char>(c.r * k),
            static_cast<unsigned char>(c.g * k),
            static_cast<unsigned char>(c.b * k)};
}

} // namespace

std::vector<Brick> makeBricks(int screenWidth, int rows, int cols) {
    std::vector<Brick> bricks;

    // Total width of the grid, so we can centre it.
    int gridWidth = cols * BRICK_W + (cols - 1) * BRICK_GAP;
    int left = (screenWidth - gridWidth) / 2;

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            Brick b;
            b.x = left + col * (BRICK_W + BRICK_GAP);
            b.y = BRICK_TOP + row * (BRICK_H + BRICK_GAP);
            b.w = BRICK_W;
            b.h = BRICK_H;
            b.color = ROW_COLORS[row % NUM_ROW_COLORS];
            b.alive = true;
            bricks.push_back(b);
        }
    }
    return bricks;
}

Paddle makePaddle(int screenWidth, int screenHeight) {
    Paddle p;
    p.w = PADDLE_W;
    p.h = PADDLE_H;
    p.x = (screenWidth - PADDLE_W) / 2.0f;
    p.y = static_cast<float>(screenHeight - PADDLE_BOTTOM - PADDLE_H);
    p.color = {220, 220, 235};
    p.squash = 0;
    return p;
}

Ball makeBall(const Paddle& paddle) {
    Ball ball;
    ball.radius = BALL_RADIUS;
    ball.cx = paddle.x + paddle.w / 2.0f;
    // Bottom of the ball (cy + radius) sits BALL_GAP pixels above the paddle.
    ball.cy = paddle.y - BALL_GAP - BALL_RADIUS - 1;
    ball.vx = 0;
    ball.vy = 0;
    ball.color = {255, 255, 255};
    ball.angle = 0;
    return ball;
}

void drawBall(Framebuffer& fb, const Ball& ball) {
    const int cx = static_cast<int>(std::lround(ball.cx));
    const int cy = static_cast<int>(std::lround(ball.cy));
    fillCircle(fb, cx, cy, ball.radius, ball.color);

    // A cross "seam" painted on the ball. It is defined once in local
    // coordinates and placed with  T(centre) * R(angle),  so the ball
    // visibly spins as it rolls.
    const Mat3 M = Mat3::translate(ball.cx, ball.cy) * Mat3::rotate(ball.angle);
    const Color seam = {170, 180, 210};
    const float len = ball.radius * 1.5f;
    fillTransformedPolygon(fb, M, centredRect(len, 3), seam);
    fillTransformedPolygon(fb, M, centredRect(3, len), seam);
}

void drawBricks(Framebuffer& fb, const std::vector<Brick>& bricks) {
    for (const Brick& b : bricks) {
        if (!b.alive) {
            continue;
        }
        fillRect(fb, b.x, b.y, b.w, b.h, b.color);
    }
}

void drawPaddle(Framebuffer& fb, const Paddle& p) {
    // An octagon (rectangle with its corners cut off), defined ONCE in
    // local coordinates centred on (0, 0).
    const float hw = p.w / 2.0f, hh = p.h / 2.0f, c = PADDLE_BEVEL;
    const std::vector<Vec2> shape = {
        {-hw + c, -hh}, { hw - c, -hh},   // top edge
        { hw, -hh + c}, { hw,  hh - c},   // right side
        { hw - c,  hh}, {-hw + c,  hh},   // bottom edge
        {-hw,  hh - c}, {-hw, -hh + c},   // left side
    };

    // Squash & stretch: when the ball lands, the paddle gets wider and
    // flatter, then springs back. Scale is anchored on the paddle's bottom
    // edge so it squashes DOWN onto the "floor" instead of shrinking in mid-air.
    const float s  = p.squash;
    const float sx = 1.0f + 0.18f * s;
    const float sy = 1.0f - 0.35f * s;

    const float centreX = p.x + hw;
    const float bottomY = p.y + p.h;

    //   world = T(centre-bottom) * S(sx, sy) * T(0, -hh)
    // The right-most matrix runs first: lift the shape so its bottom edge
    // sits on the local origin, scale, then move it into place.
    const Mat3 M = Mat3::translate(centreX, bottomY)
                 * Mat3::scale(sx, sy)
                 * Mat3::translate(0, -hh);

    fillTransformedPolygon(fb, M, shape, p.color);
}

void drawDebris(Framebuffer& fb, const Debris& d) {
    // The classic composite transform:  T * R * S
    //   1. S: shrink it as it fades (life 1 -> 0)
    //   2. R: tumble it around its own centre
    //   3. T: move it to where it has fallen to
    const Mat3 M = Mat3::translate(d.cx, d.cy)
                 * Mat3::rotate(d.angle)
                 * Mat3::scale(d.life, d.life);

    fillTransformedPolygon(fb, M, centredRect(d.w, d.h), darken(d.color, 0.5f + 0.5f * d.life));
}
