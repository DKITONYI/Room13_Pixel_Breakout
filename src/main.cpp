#include "framebuffer.h"

int main() {

    Framebuffer screen(800, 600);

    // Clear the screen with a dark background
    screen.clear({15, 15, 40});

    // Draw a red pixel
    screen.setPixel(100, 100, {255, 0, 0});

    // Draw a green pixel
    screen.setPixel(200, 200, {0, 255, 0});

    // Draw a blue pixel
    screen.setPixel(300, 300, {0, 0, 255});

    // Save the image
    screen.savePPM("test.ppm");

    return 0;
}