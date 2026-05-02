#pragma once

#include <array>
#include <cstdint>
#include <string>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

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

struct Sprite {
    Vec2 position{};
    Vec2 size{32.0f, 32.0f};
    Color color = Color::rgb(255, 255, 255);
};

class Input {
public:
    void setKey(int virtualKey, bool pressed);
    bool isKeyDown(int virtualKey) const;

private:
    std::array<bool, 256> keys_{};
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
    void drawSprite(const Sprite& sprite);
    void present(HDC deviceContext);

private:
    std::uint32_t pack(Color color) const;
    void putPixel(int x, int y, Color color);

    int width_ = 0;
    int height_ = 0;
    BITMAPINFO bitmapInfo_{};
    std::uint32_t* pixels_ = nullptr;
};

class Game {
public:
    void update(float deltaSeconds, const Input& input, Renderer& renderer);
    void render(Renderer& renderer);

private:
    Sprite player_{{120.0f, 100.0f}, {48.0f, 48.0f}, Color::rgb(245, 95, 80)};
    Sprite target_{{420.0f, 230.0f}, {72.0f, 72.0f}, Color::rgb(70, 170, 255)};
};

class Application {
public:
    Application(int width, int height, const wchar_t* title);

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    int run();

private:
    static LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT handleMessage(UINT message, WPARAM wParam, LPARAM lParam);

    int width_ = 0;
    int height_ = 0;
    HWND window_ = nullptr;
    Input input_{};
    Renderer renderer_;
    Game game_{};
    bool running_ = true;
};

} // namespace tiny2d
