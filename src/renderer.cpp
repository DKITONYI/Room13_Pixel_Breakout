#define GL_GLEXT_PROTOTYPES   // Mesa (Linux / WSLg) exports GL 3.3 functions directly
#include <GL/gl.h>
#include <GL/glext.h>

#include "renderer.h"
#include <algorithm>
#include <iostream>

namespace {

// Vertex shader: runs once per corner of the quad, on the GPU.
const char* VERTEX_SHADER = R"(
#version 330 core
layout(location = 0) in vec2 aPos;   // corner position in GAME PIXELS
layout(location = 1) in vec2 aUV;    // texture coordinate
uniform mat3 uMVP;                   // Projection * Camera
out vec2 vUV;

void main() {
    vec3 p = uMVP * vec3(aPos, 1.0); // homogeneous 2D transform
    gl_Position = vec4(p.xy, 0.0, 1.0);
    vUV = aUV;
}
)";

// Fragment shader: runs once per screen pixel, samples our framebuffer texture.
const char* FRAGMENT_SHADER = R"(
#version 330 core
in vec2 vUV;
uniform sampler2D uFrame;
out vec4 fragColor;

void main() {
    fragColor = vec4(texture(uFrame, vUV).rgb, 1.0);
}
)";

GLuint compile(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        std::cerr << "Shader compile error:\n" << log << "\n";
    }
    return s;
}

} // namespace

bool GLRenderer::init(int gameWidth, int gameHeight) {
    gameW = gameWidth;
    gameH = gameHeight;

    // --- Shaders ---
    GLuint vs = compile(GL_VERTEX_SHADER, VERTEX_SHADER);
    GLuint fs = compile(GL_FRAGMENT_SHADER, FRAGMENT_SHADER);
    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        std::cerr << "Shader link error:\n" << log << "\n";
        return false;
    }
    uMVP = glGetUniformLocation(program, "uMVP");

    // --- One quad covering the game, in game-pixel coordinates ---
    // Texture row 0 is the TOP of our framebuffer, so v = 0 at y = 0.
    const float W = static_cast<float>(gameW), H = static_cast<float>(gameH);
    const float quad[] = {
        // x  y    u  v
        0, 0,    0, 0,
        W, 0,    1, 0,
        0, H,    0, 1,
        W, H,    1, 1,
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // --- Texture that receives the framebuffer every frame ---
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);   // our pixels are tightly packed RGB
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, gameW, gameH, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    // NEAREST keeps the pixel-art edges crisp when the window is scaled.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return true;
}

void GLRenderer::viewport(int windowWidth, int windowHeight,
                          int& x, int& y, int& w, int& h) const {
    // Largest area with the game's aspect ratio that fits the window.
    const float scale = std::min(static_cast<float>(windowWidth) / gameW,
                                 static_cast<float>(windowHeight) / gameH);
    w = static_cast<int>(gameW * scale);
    h = static_cast<int>(gameH * scale);
    x = (windowWidth - w) / 2;
    y = (windowHeight - h) / 2;
}

void GLRenderer::present(const Framebuffer& fb, const Mat3& camera,
                         int windowWidth, int windowHeight) {
    // Black letterbox bars around the game.
    glViewport(0, 0, windowWidth, windowHeight);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    int vx, vy, vw, vh;
    viewport(windowWidth, windowHeight, vx, vy, vw, vh);
    glViewport(vx, vy, vw, vh);
    glEnable(GL_SCISSOR_TEST);           // keep the shake inside the game area
    glScissor(vx, vy, vw, vh);
    glClearColor(15 / 255.0f, 15 / 255.0f, 40 / 255.0f, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    // 1. CPU -> GPU: copy this frame's pixels into the texture.
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, gameW, gameH, GL_RGB, GL_UNSIGNED_BYTE, fb.data());

    // 2. Projection: game pixels (0..W, 0..H, y down) -> clip space (-1..1, y up).
    //      x' = 2x/W - 1        y' = 1 - 2y/H
    const Mat3 projection = Mat3::translate(-1, 1)
                          * Mat3::scale(2.0f / gameW, -2.0f / gameH);
    const Mat3 mvp = projection * camera;

    // 3. Draw. Our Mat3 is row-major; GL_TRUE asks OpenGL to transpose it
    //    into the column-major layout GLSL expects.
    glUseProgram(program);
    glUniformMatrix3fv(uMVP, 1, GL_TRUE, &mvp.m[0][0]);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisable(GL_SCISSOR_TEST);
}

void GLRenderer::shutdown() {
    glDeleteTextures(1, &texture);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);
}
