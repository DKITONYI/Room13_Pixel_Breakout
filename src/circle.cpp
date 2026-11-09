
#include "circle.h"

void drawCircle(Framebuffer& fb, int centerX, int centerY, int radius, Color color) {

    int x = 0;
    int y = radius;
    int p = 1 - radius;

    while (x <= y) {

        // Draw the eight symmetrical points
        fb.setPixel(centerX + x, centerY + y, color);
        fb.setPixel(centerX - x, centerY + y, color);
        fb.setPixel(centerX + x, centerY - y, color);
        fb.setPixel(centerX - x, centerY - y, color);

        fb.setPixel(centerX + y, centerY + x, color);
        fb.setPixel(centerX - y, centerY + x, color);
        fb.setPixel(centerX + y, centerY - x, color);
        fb.setPixel(centerX - y, centerY - x, color);

        x++;

        if (p < 0) {
            p = p + 2 * x + 1;
        } else {
            y--;
            p = p + 2 * (x - y) + 1;
        }
    }
}
