#ifndef TEXT_H
#define TEXT_H

#include <string>
#include "framebuffer.h"
#include "transform.h"

// Tiny 3x5 pixel font (digits + the capital letters the game uses).
// Each glyph cell is a 1x1 square in LOCAL text space; the matrix M places,
// scales and rotates the whole string, so text can grow, pulse or spin.
//
// Local text space: the string is centred on (0, 0).
void drawText(Framebuffer& fb, const std::string& text, const Mat3& M, Color color);

// Width / height of a string in local glyph cells (before M is applied).
float textWidth(const std::string& text);
const float TEXT_HEIGHT = 5.0f;

#endif
