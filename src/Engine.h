#pragma once

#include <array>
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace tiny2d {

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;
};

struct Color {
    std::uint8_t r = 255;
    std::uint8_t g = 255;
    std::uint8_t b = 255;
    std::uint8_t a = 255;

    static Color rgb(std::uint8_t red, std::uint8_t green, std::uint8_t blue) {
        return Color{red, green, blue, 255};
    }
};

struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
};

class Texture {
public:
    Texture() = default;
    Texture(int width, int height);

    static Texture createChecker(int width, int height, int cellSize, Color a, Color b);
    static Texture loadFromBmp(const std::string& path);
    static Texture loadFromPng(const std::string& path);
    static Texture loadFromFile(const std::string& path);

    int width() const { return width_; }
    int height() const { return height_; }
    bool isValid() const { return width_ > 0 && height_ > 0 && !pixels_.empty(); }

    Color pixel(int x, int y) const;
    void setPixel(int x, int y, Color color);

private:
    int width_ = 0;
    int height_ = 0;
    std::vector<Color> pixels_{};
};

struct Sprite {
    Vec2 position{};
    Vec2 size{32.0f, 32.0f};
    Color color = Color::rgb(255, 255, 255);
    const Texture* texture = nullptr;
};

class Camera2D {
public:
    explicit Camera2D(Vec2 viewportSize);

    const Vec2& position() const { return position_; }
    void setPosition(Vec2 position) { position_ = position; }

    Vec2 worldToScreen(Vec2 worldPosition) const;
    Vec2 screenToWorld(Vec2 screenPosition) const;
    void follow(Vec2 targetCenter);

private:
    Vec2 position_{};
    Vec2 viewportSize_{};
};

class Input {
public:
    enum Key {
        A = 0,
        D,
        W,
        S,
        Left,
        Right,
        Up,
        Down,
        Count
    };

    void setKey(Key key, bool pressed);
    bool isKeyDown(Key key) const;

private:
    std::array<bool, Count> keys_{};
};

class Renderer {
public:
    Renderer(int width, int height);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    int width() const { return width_; }
    int height() const { return height_; }

    void clear(Color color);
    void drawRect(Rect rect, Color color);
    void drawRect(Rect rect, Color color, const Camera2D& camera);
    void drawSprite(const Sprite& sprite);
    void drawSprite(const Sprite& sprite, const Camera2D& camera);

    const std::uint32_t* pixels() const { return pixels_; }

private:
    std::uint32_t pack(Color color) const;
    void putPixel(int x, int y, Color color);
    void drawTexture(Rect rect, const Texture& texture);

    int width_ = 0;
    int height_ = 0;
    std::uint32_t* pixels_ = nullptr;
};

class SpriteBatch {
public:
    void begin(const Camera2D& camera);
    void draw(const Sprite& sprite);
    void flush(Renderer& renderer);
    void clear();

    std::size_t size() const { return sprites_.size(); }

private:
    const Camera2D* camera_ = nullptr;
    std::vector<Sprite> sprites_{};
};

class Game {
public:
    Game();

    void update(float deltaSeconds, const Input& input, Renderer& renderer);
    void render(Renderer& renderer);

private:
    Sprite player_{{120.0f, 100.0f}, {48.0f, 48.0f}, Color::rgb(245, 95, 80)};
    Sprite target_{{920.0f, 520.0f}, {72.0f, 72.0f}, Color::rgb(70, 170, 255)};
    Camera2D camera_;
    Texture playerTexture_;
    SpriteBatch spriteBatch_;
};

class Application {
public:
    Application(int width, int height, const char* title);
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    int run();

    int width() const { return width_; }
    int height() const { return height_; }
    bool isRunning() const { return running_; }
    void stop() { running_ = false; }
    void* platformData() { return platform_; }
    Input& input() { return input_; }
    Renderer& renderer() { return renderer_; }
    Game& game() { return game_; }

private:
    int width_ = 0;
    int height_ = 0;
    void* platform_ = nullptr;
    Input input_{};
    Renderer renderer_;
    Game game_{};
    bool running_ = true;
};

} // namespace tiny2d
