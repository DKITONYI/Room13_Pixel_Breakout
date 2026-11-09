
#include "line.h"
#include <cstdlib>

void drawLine(Framebuffer& fb, int x1, int y1, int x2, int y2, Color color) {

    int dx = std::abs(x2 - x1);
    int dy = std::abs(y2 - y1);

    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;

    int error = dx - dy;

    while (true) {

        fb.setPixel(x1, y1, color);

        if (x1 == x2 && y1 == y2) {
            break;
        }

        int error2 = 2 * error;

        if (error2 > -dy) {
            error -= dy;
            x1 += sx;
        }

        if (error2 < dx) {
            error += dx;
            y1 += sy;
        }
    }
}
