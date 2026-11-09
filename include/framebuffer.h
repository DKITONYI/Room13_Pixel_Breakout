#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <vector>
#include <string>

struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

class Framebuffer {
private:
    int width;
    int height;
    std::vector<Color> pixels;

public:
    Framebuffer(int width, int height);

    void setPixel(int x, int y, Color color);
    void clear(Color color);
    void savePPM(const std::string& filename);
    void saveBMP(const std::string& filename);  // opens directly in VS Code / Windows

    int getWidth() const;
    int getHeight() const;
};

#endif