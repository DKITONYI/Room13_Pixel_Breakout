#include "text.h"

namespace {

struct Glyph {
    char ch;
    const char* rows[5]; // '#' = filled cell
};

const Glyph FONT[] = {
    {'0', {"###", "#.#", "#.#", "#.#", "###"}},
    {'1', {".#.", "##.", ".#.", ".#.", "###"}},
    {'2', {"###", "..#", "###", "#..", "###"}},
    {'3', {"###", "..#", ".##", "..#", "###"}},
    {'4', {"#.#", "#.#", "###", "..#", "..#"}},
    {'5', {"###", "#..", "###", "..#", "###"}},
    {'6', {"###", "#..", "###", "#.#", "###"}},
    {'7', {"###", "..#", "..#", ".#.", ".#."}},
    {'8', {"###", "#.#", "###", "#.#", "###"}},
    {'9', {"###", "#.#", "###", "..#", "###"}},
    {'A', {".#.", "#.#", "###", "#.#", "#.#"}},
    {'B', {"##.", "#.#", "##.", "#.#", "##."}},
    {'C', {"###", "#..", "#..", "#..", "###"}},
    {'D', {"##.", "#.#", "#.#", "#.#", "##."}},
    {'E', {"###", "#..", "##.", "#..", "###"}},
    {'F', {"###", "#..", "##.", "#..", "#.."}},
    {'G', {"###", "#..", "#.#", "#.#", "###"}},
    {'H', {"#.#", "#.#", "###", "#.#", "#.#"}},
    {'I', {"###", ".#.", ".#.", ".#.", "###"}},
    {'K', {"#.#", "#.#", "##.", "#.#", "#.#"}},
    {'L', {"#..", "#..", "#..", "#..", "###"}},
    {'M', {"#.#", "###", "###", "#.#", "#.#"}},
    {'N', {"##.", "#.#", "#.#", "#.#", "#.#"}},
    {'O', {"###", "#.#", "#.#", "#.#", "###"}},
    {'P', {"###", "#.#", "###", "#..", "#.."}},
    {'R', {"##.", "#.#", "##.", "#.#", "#.#"}},
    {'S', {"###", "#..", "###", "..#", "###"}},
    {'T', {"###", ".#.", ".#.", ".#.", ".#."}},
    {'U', {"#.#", "#.#", "#.#", "#.#", "###"}},
    {'V', {"#.#", "#.#", "#.#", "#.#", ".#."}},
    {'W', {"#.#", "#.#", "###", "###", "#.#"}},
    {'Y', {"#.#", "#.#", ".#.", ".#.", ".#."}},
    {':', {"...", ".#.", "...", ".#.", "..."}},
};

const Glyph* findGlyph(char ch) {
    for (const Glyph& g : FONT) {
        if (g.ch == ch) {
            return &g;
        }
    }
    return nullptr; // unknown characters (and spaces) draw as blank
}

const float ADVANCE = 4.0f; // 3 cells of glyph + 1 cell of spacing

} // namespace

float textWidth(const std::string& text) {
    return text.empty() ? 0.0f : text.size() * ADVANCE - 1.0f;
}

void drawText(Framebuffer& fb, const std::string& text, const Mat3& M, Color color) {
    // Shift so the string is centred on the local origin, then apply M.
    const Mat3 toWorld = M * Mat3::translate(-textWidth(text) / 2, -TEXT_HEIGHT / 2);

    // Every filled cell is a unit square pushed through the same matrix,
    // so scaling or rotating M scales/rotates the text as one object.
    static const std::vector<Vec2> unitSquare = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};

    for (size_t i = 0; i < text.size(); i++) {
        const Glyph* g = findGlyph(text[i]);
        if (!g) {
            continue;
        }
        for (int row = 0; row < 5; row++) {
            for (int col = 0; col < 3; col++) {
                if (g->rows[row][col] != '#') {
                    continue;
                }
                const Mat3 cell = toWorld * Mat3::translate(i * ADVANCE + col, static_cast<float>(row));
                fillTransformedPolygon(fb, cell, unitSquare, color);
            }
        }
    }
}
