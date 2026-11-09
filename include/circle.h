#ifndef CIRCLE_H
#define CIRCLE_H

#include "framebuffer.h"

// Draws a circle OUTLINE centred at (centerX, centerY) using the midpoint circle algorithm.
void drawCircle(Framebuffer& fb, int centerX, int centerY, int radius, Color color);

// Fills a SOLID circle centred at (cx, cy) using the midpoint circle algorithm.
// The circle is (2 * radius + 1) pixels across.
void fillCircle(Framebuffer& fb, int cx, int cy, int radius, Color color);

#endif
