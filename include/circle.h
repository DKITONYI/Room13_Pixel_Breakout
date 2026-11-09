#ifndef CIRCLE_H
#define CIRCLE_H

#include "framebuffer.h"

// Fills a solid circle centred at (cx, cy) using the midpoint circle algorithm.
// The circle is (2 * radius + 1) pixels across.
void fillCircle(Framebuffer& fb, int cx, int cy, int radius, Color color);

#endif
