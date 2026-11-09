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

namespace {

// Fills one horizontal row of pixels from xLeft to xRight (inclusive).
void drawSpan(Framebuffer& fb, int xLeft, int xRight, int y, Color color) {
    for (int x = xLeft; x <= xRight; x++) {
        fb.setPixel(x, y, color); // setPixel ignores anything off-screen
    }
}

} // namespace

void fillCircle(Framebuffer& fb, int cx, int cy, int radius, Color color) {
    if (radius < 0) {
        return;
    }

    // Same midpoint algorithm as drawCircle, but instead of plotting the
    // 8 symmetric points as dots, join each left/right pair with a
    // horizontal span so the inside gets filled.
    int x = 0;
    int y = radius;

    // Decision value: tells us whether the midpoint between the two candidate
    // next pixels is inside (d < 0) or outside (d >= 0) the true circle.
    int d = 1 - radius;

    while (x <= y) {
        drawSpan(fb, cx - x, cx + x, cy - y, color); // top cap
        drawSpan(fb, cx - x, cx + x, cy + y, color); // bottom cap
        drawSpan(fb, cx - y, cx + y, cy - x, color); // upper middle
        drawSpan(fb, cx - y, cx + y, cy + x, color); // lower middle

        if (d < 0) {
            // Midpoint is inside the circle: keep y, move right.
            d += 2 * x + 3;
        } else {
            // Midpoint is outside: move right AND step y down toward the centre.
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
}
