#ifndef POLYGON_H
#define POLYGON_H

#include <vector>
#include "framebuffer.h"

struct Point {
    int x;
    int y;
};

// Fills any simple polygon (convex or concave) using the scanline algorithm.
// Vertices are listed in order around the shape (either direction).
void fillPolygon(Framebuffer& fb, const std::vector<Point>& vertices, Color color);

// Convenience: an axis-aligned rectangle, drawn as a 4-vertex polygon.
// Covers exactly w x h pixels starting at (x, y).
void fillRect(Framebuffer& fb, int x, int y, int w, int h, Color color);

#endif
