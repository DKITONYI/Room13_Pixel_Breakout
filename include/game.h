#ifndef GAME_H
#define GAME_H

#include <deque>
#include <random>
#include <vector>
#include "breakout.h"
#include "framebuffer.h"
#include "transform.h"

enum class GameState { Ready, Playing, Paused, Won, Lost };

// What the player is doing THIS update step.
struct Input {
    bool left  = false;
    bool right = false;
    bool launch = false;      // space pressed (one-shot)
    bool hasMouse = false;    // if true, paddle follows mouseX
    float mouseX = 0;         // in game pixels
};

class Game {
public:
    Game(int width, int height);

    void reset();                                 // new game: full lives, all bricks
    void update(float dt, const Input& input);    // advance the simulation by dt seconds
    void render(Framebuffer& fb) const;           // draw the current frame (software rasterizer)

    void togglePause();
    void toggleAutoplay() { autoplay = !autoplay; }
    bool isAutoplay() const { return autoplay; }

    // Screen-shake transform for the OpenGL presentation pass (pixel space).
    Mat3 cameraTransform() const;

    int score() const { return score_; }
    int lives() const { return lives_; }
    GameState state() const { return state_; }
    float time() const { return time_; }

private:
    void serveBall();
    void loseLife();
    void movePaddle(float dt, const Input& input);
    void stepBall(float dt);
    void breakBrick(Brick& b);
    void updateDebris(float dt);

    int width, height;
    // Walls (match the lines drawn in render()).
    float wallLeft, wallRight, wallTop;

    std::vector<Brick> bricks;
    std::vector<Debris> debris;
    std::deque<Vec2> trail;   // recent ball positions for the motion trail
    Paddle paddle;
    Ball ball;

    GameState state_ = GameState::Ready;
    int score_ = 0;
    int lives_ = 3;
    float time_ = 0;          // total time since start (drives pulsing effects)
    float shakeTime = 0;      // > 0 while the screen is shaking
    float trailTimer = 0;
    bool autoplay = false;

    std::mt19937 rng;
};

#endif
