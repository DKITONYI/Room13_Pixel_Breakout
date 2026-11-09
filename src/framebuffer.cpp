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

// Writes a little-endian integer of 'bytes' bytes (BMP stores numbers this way).
static void writeLE(std::ofstream& file, unsigned int value, int bytes) {
    for (int i = 0; i < bytes; i++) {
        file.put(static_cast<char>((value >> (8 * i)) & 0xFF));
    }
}

void Framebuffer::saveBMP(const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);

    if (!file) {
        std::cerr << "Error: Could not create image file.\n";
        return;
    }

    // Each row must be padded to a multiple of 4 bytes.
    const int rowBytes   = width * 3;
    const int padding    = (4 - rowBytes % 4) % 4;
    const int imageSize  = (rowBytes + padding) * height;
    const int headerSize = 14 + 40; // file header + info header

    // --- File header (14 bytes) ---
    file.put('B');
    file.put('M');
    writeLE(file, headerSize + imageSize, 4); // total file size
    writeLE(file, 0, 4);                      // reserved
    writeLE(file, headerSize, 4);             // where pixel data starts

    // --- Info header (40 bytes) ---
    writeLE(file, 40, 4);        // size of this header
    writeLE(file, width, 4);
    writeLE(file, height, 4);    // positive = rows stored bottom-to-top
    writeLE(file, 1, 2);         // colour planes (always 1)
    writeLE(file, 24, 2);        // bits per pixel
    writeLE(file, 0, 4);         // no compression
    writeLE(file, imageSize, 4);
    writeLE(file, 2835, 4);      // horizontal resolution (72 DPI)
    writeLE(file, 2835, 4);      // vertical resolution
    writeLE(file, 0, 4);         // colours in palette (none)
    writeLE(file, 0, 4);         // important colours (all)

    // --- Pixel data: bottom row first, each pixel as B, G, R ---
    for (int y = height - 1; y >= 0; y--) {
        for (int x = 0; x < width; x++) {
            const Color& p = pixels[y * width + x];
            file.put(static_cast<char>(p.b));
            file.put(static_cast<char>(p.g));
            file.put(static_cast<char>(p.r));
        }
        for (int i = 0; i < padding; i++) {
            file.put(0);
        }
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