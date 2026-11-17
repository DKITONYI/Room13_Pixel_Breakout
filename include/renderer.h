#ifndef RENDERER_H
#define RENDERER_H

#include "framebuffer.h"
#include "transform.h"

// Presents our software-rendered Framebuffer on screen with OpenGL 3.3 core.
//
// Pipeline each frame:
//   1. Upload the CPU framebuffer into a GPU texture (glTexSubImage2D).
//   2. Draw one textured quad covering the game area.
//   3. The vertex shader moves the quad's corners with a 3x3 matrix:
//        uMVP = Projection * Camera
//      Projection maps game pixels -> OpenGL clip space (-1..1, y up).
//      Camera is the screen-shake transform (translate + rotate + zoom).
class GLRenderer {
public:
    bool init(int gameWidth, int gameHeight);
    void present(const Framebuffer& fb, const Mat3& camera, int windowWidth, int windowHeight);
    void shutdown();

    // The letterboxed area (in window pixels) the game is drawn into.
    // Used to convert mouse positions back into game pixels.
    void viewport(int windowWidth, int windowHeight,
                  int& x, int& y, int& w, int& h) const;

private:
    int gameW = 0, gameH = 0;
    unsigned int program = 0;
    unsigned int vao = 0, vbo = 0;
    unsigned int texture = 0;
    int uMVP = -1;
};

#endif
