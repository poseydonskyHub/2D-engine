#include "Engine.h"

namespace tiny2d {
namespace {

Texture loadPlayerTexture() {
    const char* paths[] = {
        "assets/player.png",
        "../assets/player.png",
        "../../assets/player.png",
        "../../../assets/player.png"
    };

    for (const char* path : paths) {
        Texture texture = Texture::loadFromFile(path);
        if (texture.isValid()) {
            return texture;
        }
    }

    return Texture{};
}

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

Game::Game()
    : camera_(Vec2{800.0f, 480.0f}),
      playerTexture_(loadPlayerTexture()) {
    if (!playerTexture_.isValid()) {
        playerTexture_ = Texture::createChecker(
            16,
            16,
            4,
            Color::rgb(245, 95, 80),
            Color::rgb(255, 205, 120)
        );
    }

    player_.size = Vec2{200.0f, 200.0f};
    player_.texture = &playerTexture_;
}

void Game::update(float deltaSeconds, const Input& input, Renderer& renderer) {
    Vec2 movement{};
    const float speed = 220.0f;

    if (input.isKeyDown(Input::A) || input.isKeyDown(Input::Left)) {
        movement.x -= 1.0f;
    }
    if (input.isKeyDown(Input::D) || input.isKeyDown(Input::Right)) {
        movement.x += 1.0f;
    }
    if (input.isKeyDown(Input::W) || input.isKeyDown(Input::Up)) {
        movement.y -= 1.0f;
    }
    if (input.isKeyDown(Input::S) || input.isKeyDown(Input::Down)) {
        movement.y += 1.0f;
    }

    player_.position.x += movement.x * speed * deltaSeconds;
    player_.position.y += movement.y * speed * deltaSeconds;

    player_.position.x = clampFloat(player_.position.x, 0.0f, 1600.0f - player_.size.x);
    player_.position.y = clampFloat(player_.position.y, 0.0f, 960.0f - player_.size.y);

    const Vec2 playerCenter{
        player_.position.x + player_.size.x * 0.5f,
        player_.position.y + player_.size.y * 0.5f
    };
    camera_.follow(playerCenter);
    camera_.setPosition(Vec2{
        clampFloat(camera_.position().x, 0.0f, 1600.0f - static_cast<float>(renderer.width())),
        clampFloat(camera_.position().y, 0.0f, 960.0f - static_cast<float>(renderer.height()))
    });
}

void Game::render(Renderer& renderer) {
    renderer.drawRect(Rect{0.0f, 0.0f, 1600.0f, 960.0f}, Color::rgb(30, 34, 42), camera_);

    for (int x = 0; x <= 1600; x += 32) {
        renderer.drawRect(Rect{static_cast<float>(x), 0.0f, 1.0f, 960.0f}, Color::rgb(44, 49, 58), camera_);
    }

    for (int y = 0; y <= 960; y += 32) {
        renderer.drawRect(Rect{0.0f, static_cast<float>(y), 1600.0f, 1.0f}, Color::rgb(44, 49, 58), camera_);
    }

    // SpiteBatch
    spriteBatch_.begin(camera_);
    spriteBatch_.draw(target_);
    spriteBatch_.draw(player_);
    spriteBatch_.flush(renderer);
}

} // namespace tiny2d
