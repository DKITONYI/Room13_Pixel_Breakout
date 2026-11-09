#include "framebuffer.h"
#include <fstream>
#include <iostream>

Framebuffer::Framebuffer(int width, int height)
    : width(width), height(height),
      pixels(width * height, {0, 0, 0}) {
}

void Framebuffer::setPixel(int x, int y, Color color) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }

    pixels[y * width + x] = color;
}

void Framebuffer::clear(Color color) {
    for (int i = 0; i < width * height; i++) {
        pixels[i] = color;
    }
}

void Framebuffer::savePPM(const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);

    if (!file) {
        std::cerr << "Error: Could not create image file.\n";
        return;
    }

    file << "P6\n" << width << " " << height << "\n255\n";

    for (const Color& pixel : pixels) {
        file.write(reinterpret_cast<const char*>(&pixel), 3);
    }

    file.close();

    std::cout << "Image saved successfully: " << filename << "\n";
}

int Framebuffer::getWidth() const {
    return width;
}

int Framebuffer::getHeight() const {
    return height;
}