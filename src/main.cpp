#include "framebuffer.h"
#include "breakout.h"
#include "line.h"

int main() {
    const int WIDTH  = 800;
    const int HEIGHT = 600;

    Framebuffer screen(WIDTH, HEIGHT);

    // Dark background
    screen.clear({15, 15, 40});

    // Game walls - white (Bresenham lines). Placed just outside the brick grid.
    const Color WALL = {255, 255, 255};
    drawLine(screen, 16, 40, WIDTH - 17, 40, WALL);               // top
    drawLine(screen, 16, 40, 16, HEIGHT - 1, WALL);               // left
    drawLine(screen, WIDTH - 17, 40, WIDTH - 17, HEIGHT - 1, WALL); // right

    // Build the game objects
    std::vector<Brick> bricks = makeBricks(WIDTH, 6, 10);
    Paddle paddle = makePaddle(WIDTH, HEIGHT);
    Ball ball = makeBall(paddle);

    // Draw them
    drawBricks(screen, bricks);
    drawPaddle(screen, paddle);
    drawBall(screen, ball);

    screen.savePPM("breakout.ppm");
    screen.saveBMP("breakout.bmp");  // click this one in VS Code to view it

    return 0;
}
