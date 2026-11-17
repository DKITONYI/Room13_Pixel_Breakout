#include "game.h"
#include "circle.h"
#include "line.h"
#include "text.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace {

const int   BRICK_ROWS = 6;
const int   BRICK_COLS = 10;

const float PI               = 3.14159265f;
const float PADDLE_SPEED     = 650;   // px/s with the keyboard
const float AUTOPLAY_SPEED   = 1100;  // px/s for the demo autopilot
const float BALL_START_SPEED = 380;   // px/s
const float BALL_MAX_SPEED   = 700;
const float BALL_SPEEDUP     = 6;     // added per brick broken
const float MAX_BOUNCE_ANGLE = 60 * PI / 180; // off vertical, at the paddle's edge
const float GRAVITY          = 900;   // px/s^2, pulls debris down
const float SQUASH_RECOVERY  = 4;     // squash goes 1 -> 0 in 0.25 s
const float SHAKE_DURATION   = 0.18f; // seconds
const float SHAKE_PIXELS     = 6;
const size_t TRAIL_LENGTH    = 10;
const float TRAIL_INTERVAL   = 1.0f / 60;

const Color BACKGROUND = {15, 15, 40};
const Color WALL       = {255, 255, 255};
const Color HUD        = {200, 205, 230};
const Color TRAIL      = {120, 150, 255};

bool circleHitsRect(float cx, float cy, float r, float x, float y, float w, float h) {
    // Closest point on the rectangle to the circle's centre.
    const float nx = std::clamp(cx, x, x + w);
    const float ny = std::clamp(cy, y, y + h);
    const float dx = cx - nx, dy = cy - ny;
    return dx * dx + dy * dy <= r * r;
}

Color mix(Color a, Color b, float t) {
    auto ch = [t](unsigned char p, unsigned char q) {
        return static_cast<unsigned char>(p + (q - p) * t);
    };
    return {ch(a.r, b.r), ch(a.g, b.g), ch(a.b, b.b)};
}

} // namespace

Game::Game(int width, int height)
    : width(width), height(height),
      wallLeft(16), wallRight(static_cast<float>(width - 17)), wallTop(40),
      rng(std::random_device{}()) {
    reset();
}

void Game::reset() {
    bricks = makeBricks(width, BRICK_ROWS, BRICK_COLS);
    paddle = makePaddle(width, height);
    ball = makeBall(paddle);
    debris.clear();
    trail.clear();
    score_ = 0;
    lives_ = 3;
    shakeTime = 0;
    state_ = GameState::Ready;
}

void Game::togglePause() {
    if (state_ == GameState::Playing) {
        state_ = GameState::Paused;
    } else if (state_ == GameState::Paused) {
        state_ = GameState::Playing;
    }
}

// ---------------------------------------------------------------------------
// Simulation
// ---------------------------------------------------------------------------

void Game::update(float dt, const Input& input) {
    if (state_ == GameState::Paused) {
        return; // freeze everything
    }
    time_ += dt;

    if (state_ == GameState::Won || state_ == GameState::Lost) {
        updateDebris(dt);
        shakeTime = std::max(0.0f, shakeTime - dt);
        if (input.launch) {
            reset();
        }
        return;
    }

    movePaddle(dt, input);
    paddle.squash = std::max(0.0f, paddle.squash - SQUASH_RECOVERY * dt);

    if (state_ == GameState::Ready) {
        // Ball rides on top of the paddle until it is launched.
        ball.cx = paddle.x + paddle.w / 2.0f;
        if (input.launch || autoplay) {
            serveBall();
        }
    } else if (state_ == GameState::Playing) {
        stepBall(dt);

        trailTimer += dt;
        if (trailTimer >= TRAIL_INTERVAL) {
            trailTimer = 0;
            trail.push_back({ball.cx, ball.cy});
            if (trail.size() > TRAIL_LENGTH) {
                trail.pop_front();
            }
        }
    }

    updateDebris(dt);
    shakeTime = std::max(0.0f, shakeTime - dt);
}

void Game::serveBall() {
    std::uniform_real_distribution<float> angle(-PI / 6, PI / 6);
    const float a = angle(rng);
    ball.vx = BALL_START_SPEED * std::sin(a);
    ball.vy = -BALL_START_SPEED * std::cos(a);
    state_ = GameState::Playing;
}

void Game::movePaddle(float dt, const Input& input) {
    const float half = paddle.w / 2.0f;
    float centre = paddle.x + half;

    if (autoplay) {
        // Demo autopilot: chase the ball, aiming slightly off-centre
        // (swaying over time) so the bounce angle keeps changing.
        const float target = ball.cx + std::sin(time_ * 1.3f) * half * 0.6f;
        const float step = AUTOPLAY_SPEED * dt;
        centre += std::clamp(target - centre, -step, step);
    } else if (input.hasMouse) {
        centre = input.mouseX;
    } else {
        const float dir = (input.right ? 1.0f : 0.0f) - (input.left ? 1.0f : 0.0f);
        centre += dir * PADDLE_SPEED * dt;
    }

    paddle.x = std::clamp(centre - half, wallLeft + 1, wallRight - paddle.w);
}

void Game::stepBall(float dt) {
    const float r = static_cast<float>(ball.radius);
    const float prevX = ball.cx, prevY = ball.cy;

    // Translation: position += velocity * time.
    ball.cx += ball.vx * dt;
    ball.cy += ball.vy * dt;

    // Rolling: a ball that moves a distance d turns by d / r radians.
    ball.angle += ball.vx / r * dt;

    // --- Walls ---
    if (ball.cx - r < wallLeft + 1) {
        ball.cx = wallLeft + 1 + r;
        ball.vx = std::fabs(ball.vx);
    }
    if (ball.cx + r > wallRight) {
        ball.cx = wallRight - r;
        ball.vx = -std::fabs(ball.vx);
    }
    if (ball.cy - r < wallTop + 1) {
        ball.cy = wallTop + 1 + r;
        ball.vy = std::fabs(ball.vy);
    }

    // --- Paddle ---
    if (ball.vy > 0 && prevY < paddle.y + paddle.h / 2.0f &&
        circleHitsRect(ball.cx, ball.cy, r, paddle.x, paddle.y,
                       static_cast<float>(paddle.w), static_cast<float>(paddle.h))) {
        // Where on the paddle did it land? -1 = left edge, 0 = centre, +1 = right edge.
        const float half = paddle.w / 2.0f;
        const float offset = std::clamp((ball.cx - (paddle.x + half)) / (half + r), -1.0f, 1.0f);

        // Bounce angle depends on the hit point; speed is preserved.
        const float speed = std::hypot(ball.vx, ball.vy);
        const float angle = offset * MAX_BOUNCE_ANGLE;
        ball.vx = speed * std::sin(angle);
        ball.vy = -speed * std::cos(angle);
        ball.cy = paddle.y - r - 1;
        paddle.squash = 1; // trigger squash & stretch
    }

    // --- Bricks (at most one per step) ---
    for (Brick& b : bricks) {
        if (!b.alive || !circleHitsRect(ball.cx, ball.cy, r, static_cast<float>(b.x),
                                        static_cast<float>(b.y), static_cast<float>(b.w),
                                        static_cast<float>(b.h))) {
            continue;
        }

        // Which side did we come from? Use the position before this step.
        const bool cameFromAboveOrBelow = prevX >= b.x && prevX <= b.x + b.w;
        const bool cameFromSide         = prevY >= b.y && prevY <= b.y + b.h;
        if (cameFromAboveOrBelow || !cameFromSide) {
            ball.vy = -ball.vy;
        }
        if (cameFromSide || !cameFromAboveOrBelow) {
            ball.vx = -ball.vx;
        }
        ball.cx = prevX;
        ball.cy = prevY;

        breakBrick(b);

        // Each brick makes the ball a little faster.
        const float speed = std::hypot(ball.vx, ball.vy);
        const float newSpeed = std::min(speed + BALL_SPEEDUP, BALL_MAX_SPEED);
        ball.vx *= newSpeed / speed;
        ball.vy *= newSpeed / speed;
        break;
    }

    // --- Fell off the bottom ---
    if (ball.cy - r > height) {
        loseLife();
    }
}

void Game::breakBrick(Brick& b) {
    b.alive = false;
    score_ += 10;
    shakeTime = SHAKE_DURATION;

    // Turn the brick into tumbling debris.
    std::uniform_real_distribution<float> jitter(-60, 60);
    std::uniform_real_distribution<float> pop(-220, -120);
    std::uniform_real_distribution<float> spin(-8, 8);

    Debris d;
    d.cx = b.x + b.w / 2.0f;
    d.cy = b.y + b.h / 2.0f;
    d.vx = ball.vx * 0.25f + jitter(rng);
    d.vy = pop(rng);
    d.angle = 0;
    d.spin = spin(rng);
    d.life = 1;
    d.w = b.w;
    d.h = b.h;
    d.color = b.color;
    debris.push_back(d);

    const bool anyLeft = std::any_of(bricks.begin(), bricks.end(),
                                     [](const Brick& br) { return br.alive; });
    if (!anyLeft) {
        state_ = GameState::Won;
    }
}

void Game::loseLife() {
    lives_--;
    shakeTime = SHAKE_DURATION * 2; // bigger shake when you lose a ball
    trail.clear();
    ball = makeBall(paddle);
    state_ = (lives_ <= 0) ? GameState::Lost : GameState::Ready;
}

void Game::updateDebris(float dt) {
    for (Debris& d : debris) {
        d.vy += GRAVITY * dt;
        d.cx += d.vx * dt;
        d.cy += d.vy * dt;
        d.angle += d.spin * dt;
        d.life -= 0.9f * dt;
    }
    debris.erase(std::remove_if(debris.begin(), debris.end(),
                                [](const Debris& d) { return d.life <= 0; }),
                 debris.end());
}

// ---------------------------------------------------------------------------
// Rendering (all on the CPU with the team's own rasterization algorithms)
// ---------------------------------------------------------------------------

void Game::render(Framebuffer& fb) const {
    fb.clear(BACKGROUND);

    // Walls (Bresenham lines).
    const int L = static_cast<int>(wallLeft), R = static_cast<int>(wallRight);
    const int T = static_cast<int>(wallTop);
    drawLine(fb, L, T, R, T, WALL);
    drawLine(fb, L, T, L, height - 1, WALL);
    drawLine(fb, R, T, R, height - 1, WALL);

    drawBricks(fb, bricks);
    for (const Debris& d : debris) {
        drawDebris(fb, d);
    }

    // Motion trail: older positions are smaller and closer to the background colour.
    for (size_t i = 0; i < trail.size(); i++) {
        const float t = (i + 1.0f) / (trail.size() + 1.0f);
        const int r = static_cast<int>(ball.radius * 0.8f * t);
        fillCircle(fb, static_cast<int>(std::lround(trail[i].x)),
                   static_cast<int>(std::lround(trail[i].y)), r, mix(BACKGROUND, TRAIL, 0.6f * t));
    }

    drawPaddle(fb, paddle);
    drawBall(fb, ball);

    // --- HUD --- (3x5 font, scaled 3x by the matrix)
    const std::string scoreText = "SCORE " + std::to_string(score_);
    const float hudScale = 3;
    drawText(fb, scoreText,
             Mat3::translate(wallLeft + textWidth(scoreText) * hudScale / 2, 20) *
             Mat3::scale(hudScale, hudScale),
             HUD);

    for (int i = 0; i < lives_; i++) {
        fillCircle(fb, static_cast<int>(wallRight) - 10 - i * 22, 20, 7, {255, 255, 255});
    }

    if (autoplay) {
        drawText(fb, "AUTO", Mat3::translate(width / 2.0f, 20) * Mat3::scale(3, 3), {120, 220, 140});
    }

    // --- Centre messages: scale and rotate transforms on the text ---
    const float cx = width / 2.0f;
    const float cy = 360;
    const float pulse = 1.0f + 0.12f * std::sin(time_ * 5); // breathing scale

    switch (state_) {
    case GameState::Ready:
        drawText(fb, "PRESS SPACE",
                 Mat3::translate(cx, cy) * Mat3::scale(5 * pulse, 5 * pulse), {255, 255, 255});
        break;
    case GameState::Paused:
        drawText(fb, "PAUSED", Mat3::translate(cx, cy) * Mat3::scale(8, 8), {255, 255, 255});
        break;
    case GameState::Won:
    case GameState::Lost: {
        const bool won = state_ == GameState::Won;
        // Wobble: a small rotation that swings back and forth.
        const Mat3 M = Mat3::translate(cx, cy - 30)
                     * Mat3::rotate(0.08f * std::sin(time_ * 3))
                     * Mat3::scale(9 * pulse, 9 * pulse);
        drawText(fb, won ? "YOU WIN" : "GAME OVER", M,
                 won ? Color{120, 230, 140} : Color{240, 90, 100});
        drawText(fb, "PRESS SPACE", Mat3::translate(cx, cy + 40) * Mat3::scale(3, 3), HUD);
        break;
    }
    case GameState::Playing:
        break;
    }
}

Mat3 Game::cameraTransform() const {
    if (shakeTime <= 0) {
        return Mat3::identity();
    }
    // k fades the shake out as shakeTime runs down.
    const float k  = shakeTime / SHAKE_DURATION;
    const float dx = SHAKE_PIXELS * k * std::sin(time_ * 90);
    const float dy = SHAKE_PIXELS * k * std::cos(time_ * 73);
    const float rot  = 0.01f * k * std::sin(time_ * 60);
    const float zoom = 1.0f + 0.02f * k;   // zoom in a touch so the edges stay covered

    const float px = width / 2.0f, py = height / 2.0f;
    return Mat3::translate(dx, dy)
         * aboutPoint(Mat3::rotate(rot) * Mat3::scale(zoom, zoom), px, py);
}
