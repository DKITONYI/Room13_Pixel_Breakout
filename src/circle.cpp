#include "circle.h"

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

    // Start at the top of the circle's 1/8 arc: straight up from the centre,
    // and step x to the right. We stop once we reach the 45-degree diagonal
    // (x > y); the other 7 octants are mirror images.
    int x = 0;
    int y = radius;

    // Decision value: tells us whether the midpoint between the two candidate
    // next pixels is inside (d < 0) or outside (d >= 0) the true circle.
    int d = 1 - radius;

    while (x <= y) {
        // Each (x, y) on the arc gives 8 symmetric points. Instead of plotting
        // them as dots, join the left/right pairs with horizontal spans.
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
