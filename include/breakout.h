#ifndef BREAKOUT_H
#define BREAKOUT_H

#include <vector>
#include "framebuffer.h"

struct Brick {
    int x, y;       // top-left corner
    int w, h;
    Color color;
    bool alive;     // false once the ball has destroyed it
};

struct Paddle {
    int x, y;       // top-left corner
    int w, h;
    Color color;
};

struct Ball {
    int cx, cy;     // centre
    int radius;
    Color color;
};

// Lays out a rows x cols grid of bricks, centred horizontally.
std::vector<Brick> makeBricks(int screenWidth, int rows, int cols);

// Creates a paddle centred near the bottom of the screen.
Paddle makePaddle(int screenWidth, int screenHeight);

// Creates a ball resting just above the middle of the paddle.
Ball makeBall(const Paddle& paddle);

void drawBricks(Framebuffer& fb, const std::vector<Brick>& bricks);
void drawPaddle(Framebuffer& fb, const Paddle& paddle);
void drawBall(Framebuffer& fb, const Ball& ball);

#endif
