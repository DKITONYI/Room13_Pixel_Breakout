#include "polygon.h"
#include <algorithm>
#include <cmath>

namespace {

// One non-horizontal polygon edge, stored top-to-bottom.
struct Edge {
    double yTop;     // smaller y (screen y grows downward)
    double yBottom;  // larger y
    double xAtTop;   // x where the edge starts at yTop
    double dxdy;     // inverse slope: how much x moves per 1 step in y
};

} // namespace

void fillPolygon(Framebuffer& fb, const std::vector<Point>& vertices, Color color) {
    const size_t n = vertices.size();
    if (n < 3) {
        return; // not a polygon
    }

    // 1. Build the edge list and find the polygon's vertical extent.
    std::vector<Edge> edges;
    int minY = vertices[0].y;
    int maxY = vertices[0].y;

    for (size_t i = 0; i < n; i++) {
        Point a = vertices[i];
        Point b = vertices[(i + 1) % n]; // wrap around to close the shape

        minY = std::min(minY, a.y);
        maxY = std::max(maxY, a.y);

        if (a.y == b.y) {
            continue; // horizontal edges never cross a scanline; skip them
        }
        if (a.y > b.y) {
            std::swap(a, b); // make 'a' the top end
        }

        edges.push_back({
            static_cast<double>(a.y),
            static_cast<double>(b.y),
            static_cast<double>(a.x),
            static_cast<double>(b.x - a.x) / (b.y - a.y)
        });
    }

    // Don't waste time on rows that are off-screen.
    minY = std::max(minY, 0);
    maxY = std::min(maxY, fb.getHeight());

    // 2. Walk down the polygon one scanline (pixel row) at a time.
    std::vector<double> crossings;
    for (int y = minY; y < maxY; y++) {
        const double sampleY = y + 0.5; // test through the pixel's centre

        // 3. Find where this scanline crosses each edge.
        //    Half-open test [yTop, yBottom) so a shared vertex is
        //    counted once, not twice.
        crossings.clear();
        for (const Edge& e : edges) {
            if (sampleY >= e.yTop && sampleY < e.yBottom) {
                crossings.push_back(e.xAtTop + (sampleY - e.yTop) * e.dxdy);
            }
        }

        // 4. Sort left to right, then fill between pairs:
        //    outside -> [inside] -> outside -> [inside] ...
        std::sort(crossings.begin(), crossings.end());

        for (size_t i = 0; i + 1 < crossings.size(); i += 2) {
            // Fill pixels whose centres (x + 0.5) lie in [left, right).
            int xStart = static_cast<int>(std::ceil(crossings[i] - 0.5));
            int xEnd   = static_cast<int>(std::ceil(crossings[i + 1] - 0.5));

            xStart = std::max(xStart, 0);
            xEnd   = std::min(xEnd, fb.getWidth());

            for (int x = xStart; x < xEnd; x++) {
                fb.setPixel(x, y, color);
            }
        }
    }
}

void fillRect(Framebuffer& fb, int x, int y, int w, int h, Color color) {
    fillPolygon(fb, {
        {x,     y},
        {x + w, y},
        {x + w, y + h},
        {x,     y + h}
    }, color);
}
