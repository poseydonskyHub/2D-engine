#include "Engine.h"

namespace tiny2d {
namespace {

float clampFloat(float value, float minimum, float maximum) {
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }
    return value;
}

} // namespace

void Game::update(float deltaSeconds, const Input& input, Renderer& renderer) {
    Vec2 movement{};
    const float speed = 220.0f;

    if (input.isKeyDown('A') || input.isKeyDown(VK_LEFT)) {
        movement.x -= 1.0f;
    }
    if (input.isKeyDown('D') || input.isKeyDown(VK_RIGHT)) {
        movement.x += 1.0f;
    }
    if (input.isKeyDown('W') || input.isKeyDown(VK_UP)) {
        movement.y -= 1.0f;
    }
    if (input.isKeyDown('S') || input.isKeyDown(VK_DOWN)) {
        movement.y += 1.0f;
    }

    player_.position.x += movement.x * speed * deltaSeconds;
    player_.position.y += movement.y * speed * deltaSeconds;

    player_.position.x = clampFloat(player_.position.x, 0.0f, static_cast<float>(renderer.width()) - player_.size.x);
    player_.position.y = clampFloat(player_.position.y, 0.0f, static_cast<float>(renderer.height()) - player_.size.y);
}

void Game::render(Renderer& renderer) {
    for (int x = 0; x < renderer.width(); x += 32) {
        renderer.drawRect(Rect{static_cast<float>(x), 0.0f, 1.0f, static_cast<float>(renderer.height())}, Color::rgb(38, 42, 50));
    }

    for (int y = 0; y < renderer.height(); y += 32) {
        renderer.drawRect(Rect{0.0f, static_cast<float>(y), static_cast<float>(renderer.width()), 1.0f}, Color::rgb(38, 42, 50));
    }

    renderer.drawSprite(target_);
    renderer.drawSprite(player_);
}

} // namespace tiny2d
