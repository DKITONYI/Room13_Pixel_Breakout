
#include "framebuffer.h"
#include "line.h"
#include "circle.h"

int main() {

    Framebuffer screen(800, 600);

    // Background
    screen.clear({15, 15, 40});

    // Game walls - white
    drawLine(screen, 50, 50, 750, 50, {255, 255, 255});
    drawLine(screen, 50, 50, 50, 550, {255, 255, 255});
    drawLine(screen, 750, 50, 750, 550, {255, 255, 255});

    // Bricks - different colours

    // First row - red
    drawLine(screen, 100, 100, 220, 100, {255, 0, 0});
    drawLine(screen, 220, 100, 220, 140, {255, 0, 0});
    drawLine(screen, 220, 140, 100, 140, {255, 0, 0});
    drawLine(screen, 100, 140, 100, 100, {255, 0, 0});

    drawLine(screen, 240, 100, 360, 100, {255, 0, 0});
    drawLine(screen, 360, 100, 360, 140, {255, 0, 0});
    drawLine(screen, 360, 140, 240, 140, {255, 0, 0});
    drawLine(screen, 240, 140, 240, 100, {255, 0, 0});

    // Second row - yellow
    drawLine(screen, 100, 160, 220, 160, {255, 255, 0});
    drawLine(screen, 220, 160, 220, 200, {255, 255, 0});
    drawLine(screen, 220, 200, 100, 200, {255, 255, 0});
    drawLine(screen, 100, 200, 100, 160, {255, 255, 0});

    drawLine(screen, 240, 160, 360, 160, {255, 255, 0});
    drawLine(screen, 360, 160, 360, 200, {255, 255, 0});
    drawLine(screen, 360, 200, 240, 200, {255, 255, 0});
    drawLine(screen, 240, 200, 240, 160, {255, 255, 0});

    // Paddle - green
    drawLine(screen, 320, 500, 480, 500, {0, 255, 0});
    drawLine(screen, 320, 510, 480, 510, {0, 255, 0});
    drawLine(screen, 320, 500, 320, 510, {0, 255, 0});
    drawLine(screen, 480, 500, 480, 510, {0, 255, 0});

    // Ball - white
    drawCircle(screen, 400, 450, 12, {255, 255, 255});

    // Save game scene
    screen.savePPM("game_scene.ppm");

    return 0;
}
