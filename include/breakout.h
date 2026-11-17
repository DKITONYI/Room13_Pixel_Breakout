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
    float x, y;     // top-left corner (float so it can move smoothly)
    int w, h;
    Color color;
    float squash;   // 1 right after the ball hits, decays to 0 (squash & stretch)
};

struct Ball {
    float cx, cy;   // centre
    float vx, vy;   // velocity in pixels per second
    int radius;
    Color color;
    float angle;    // spin angle in radians (the ball "rolls" as it moves)
};

// A destroyed brick: tumbles, shrinks and falls off-screen.
struct Debris {
    float cx, cy;   // centre
    float vx, vy;   // velocity in pixels per second
    float angle;    // current rotation
    float spin;     // rotation speed, radians per second
    float life;     // 1 -> 0; also used as the scale factor
    int w, h;
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
void drawDebris(Framebuffer& fb, const Debris& d);

#endif
