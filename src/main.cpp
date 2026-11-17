#include "framebuffer.h"
#include "game.h"
#include "renderer.h"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {

const int WIDTH  = 800;
const int HEIGHT = 600;
const double FIXED_DT = 1.0 / 120.0;  // physics runs at a steady 120 Hz

// Everything the GLFW callbacks need to reach.
struct App {
    Game* game = nullptr;
    Framebuffer* fb = nullptr;
    GLRenderer* renderer = nullptr;
    bool launchPressed = false;
    bool mouseActive = false;
    float mouseX = 0;
    int screenshots = 0;
};

void onKey(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
    if (action != GLFW_PRESS) {
        return;
    }
    switch (key) {
    case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
    case GLFW_KEY_SPACE:  app->launchPressed = true; break;
    case GLFW_KEY_P:      app->game->togglePause(); break;
    case GLFW_KEY_R:      app->game->reset(); break;
    case GLFW_KEY_A:      app->game->toggleAutoplay(); break;
    case GLFW_KEY_LEFT:
    case GLFW_KEY_RIGHT:  app->mouseActive = false; break; // keyboard takes over again
    case GLFW_KEY_F12:
    case GLFW_KEY_S: {
        char name[64];
        std::snprintf(name, sizeof(name), "screenshot_%02d.bmp", app->screenshots++);
        app->fb->saveBMP(name);
        break;
    }
    default: break;
    }
}

void onMouseMove(GLFWwindow* window, double x, double /*y*/) {
    App* app = static_cast<App*>(glfwGetWindowUserPointer(window));

    // Window coordinates -> game pixels (undo the letterbox scaling).
    // Window size and framebuffer size can differ on high-DPI screens.
    int winW, winH, fbW, fbH, vx, vy, vw, vh;
    glfwGetWindowSize(window, &winW, &winH);
    glfwGetFramebufferSize(window, &fbW, &fbH);
    app->renderer->viewport(fbW, fbH, vx, vy, vw, vh);
    const double px = x * fbW / std::max(winW, 1);
    app->mouseX = static_cast<float>((px - vx) * WIDTH / std::max(vw, 1));
    app->mouseActive = true;
}

// Headless mode: run the game on autopilot and dump frames as BMP images.
// Useful as animation evidence (stitch into a GIF/video) and needs no window.
int recordFrames(const std::string& dir, int count) {
    Game game(WIDTH, HEIGHT);
    Framebuffer fb(WIDTH, HEIGHT);
    game.toggleAutoplay();

    const int stepsPerFrame = 4; // 120 Hz physics / 4 = 30 frames per second
    for (int f = 0; f < count; f++) {
        for (int s = 0; s < stepsPerFrame; s++) {
            game.update(static_cast<float>(FIXED_DT), Input{});
        }
        game.render(fb);
        char name[512];
        std::snprintf(name, sizeof(name), "%s/frame_%04d.bmp", dir.c_str(), f);
        fb.saveBMP(name);
    }
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    if (argc >= 3 && std::string(argv[1]) == "--frames") {
        const int count = (argc >= 4) ? std::atoi(argv[3]) : 90;
        return recordFrames(argv[2], count);
    }

    if (!glfwInit()) {
        std::cerr << "Failed to initialise GLFW\n";
        return 1;
    }

    // Ask for a modern (core profile) OpenGL 3.3 context.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Pixel Breakout", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create an OpenGL 3.3 window\n";
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync

    std::cout << "OpenGL " << glGetString(GL_VERSION) << " | " << glGetString(GL_RENDERER) << "\n"
              << "Controls: Left/Right or mouse = move, Space = launch, P = pause,\n"
              << "          R = restart, A = autoplay, S/F12 = screenshot, Esc = quit" << std::endl;

    Game game(WIDTH, HEIGHT);
    Framebuffer fb(WIDTH, HEIGHT);
    GLRenderer renderer;
    if (!renderer.init(WIDTH, HEIGHT)) {
        glfwTerminate();
        return 1;
    }

    App app;
    app.game = &game;
    app.fb = &fb;
    app.renderer = &renderer;
    glfwSetWindowUserPointer(window, &app);
    glfwSetKeyCallback(window, onKey);
    glfwSetCursorPosCallback(window, onMouseMove);

    // Game loop with a fixed time step: rendering speed can vary,
    // but the physics always advances in identical 1/120 s steps.
    double previous = glfwGetTime();
    double accumulator = 0;
    double titleTimer = 0;
    int framesThisSecond = 0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        const double now = glfwGetTime();
        const double frameTime = std::min(now - previous, 0.25); // avoid a "spiral of death"
        previous = now;
        accumulator += frameTime;

        Input input;
        input.left   = glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS;
        input.right  = glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS;
        input.hasMouse = app.mouseActive;
        input.mouseX = app.mouseX;
        input.launch = app.launchPressed;
        app.launchPressed = false;

        while (accumulator >= FIXED_DT) {
            game.update(static_cast<float>(FIXED_DT), input);
            input.launch = false; // a key press only counts once
            accumulator -= FIXED_DT;
        }

        game.render(fb);                       // CPU: our rasterizer draws the frame

        int fbW, fbH;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        renderer.present(fb, game.cameraTransform(), fbW, fbH); // GPU: show it
        glfwSwapBuffers(window);

        // FPS + score in the title bar.
        framesThisSecond++;
        titleTimer += frameTime;
        if (titleTimer >= 0.5) {
            char title[128];
            std::snprintf(title, sizeof(title), "Pixel Breakout | Score %d | Lives %d | %.0f FPS",
                          game.score(), game.lives(), framesThisSecond / titleTimer);
            glfwSetWindowTitle(window, title);
            titleTimer = 0;
            framesThisSecond = 0;
        }
    }

    renderer.shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
