#include "breakout.h"
#include "polygon.h"
#include "circle.h"

namespace {

// Layout constants (pixels).
const int BRICK_W   = 68;
const int BRICK_H   = 22;
const int BRICK_GAP = 6;
const int BRICK_TOP = 60;   // distance from top of screen to first row

const int PADDLE_W      = 120;
const int PADDLE_H      = 16;
const int PADDLE_BOTTOM = 40;  // gap between paddle and bottom of screen
const int PADDLE_BEVEL  = 6;   // size of the cut-off corners

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
    p.x = (screenWidth - PADDLE_W) / 2;
    p.y = screenHeight - PADDLE_BOTTOM - PADDLE_H;
    p.color = {220, 220, 235};
    return p;
}

Ball makeBall(const Paddle& paddle) {
    Ball ball;
    ball.radius = BALL_RADIUS;
    ball.cx = paddle.x + paddle.w / 2;
    // Bottom of the ball (cy + radius) sits BALL_GAP pixels above the paddle.
    ball.cy = paddle.y - BALL_GAP - BALL_RADIUS - 1;
    ball.color = {255, 255, 255};
    return ball;
}

void drawBall(Framebuffer& fb, const Ball& ball) {
    fillCircle(fb, ball.cx, ball.cy, ball.radius, ball.color);
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
    // An octagon: a rectangle with its four corners cut off.
    // Uses the general polygon fill, not just fillRect.
    const int c = PADDLE_BEVEL;
    const int x0 = p.x, x1 = p.x + p.w;
    const int y0 = p.y, y1 = p.y + p.h;

    fillPolygon(fb, {
        {x0 + c, y0},      // top edge
        {x1 - c, y0},
        {x1,     y0 + c},  // right side
        {x1,     y1 - c},
        {x1 - c, y1},      // bottom edge
        {x0 + c, y1},
        {x0,     y1 - c},  // left side
        {x0,     y0 + c},
    }, p.color);
}
