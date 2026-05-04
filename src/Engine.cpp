#include "Engine.h"

#include <algorithm>
#include <cctype>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "../third_party/stb_image.h"

namespace tiny2d {
namespace {

std::uint16_t readU16(std::ifstream& file) {
    std::uint8_t bytes[2]{};
    file.read(reinterpret_cast<char*>(bytes), 2);
    return static_cast<std::uint16_t>(bytes[0] | (bytes[1] << 8));
}

std::uint32_t readU32(std::ifstream& file) {
    std::uint8_t bytes[4]{};
    file.read(reinterpret_cast<char*>(bytes), 4);
    return static_cast<std::uint32_t>(bytes[0])
        | (static_cast<std::uint32_t>(bytes[1]) << 8)
        | (static_cast<std::uint32_t>(bytes[2]) << 16)
        | (static_cast<std::uint32_t>(bytes[3]) << 24);
}

std::int32_t readI32(std::ifstream& file) {
    return static_cast<std::int32_t>(readU32(file));
}

std::string lowerCase(std::string text) {
    for (char& character : text) {
        character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    }
    return text;
}

std::string fileExtension(const std::string& path) {
    const std::size_t dot = path.find_last_of('.');
    if (dot == std::string::npos) {
        return "";
    }
    return lowerCase(path.substr(dot));
}

} // namespace

Texture::Texture(int width, int height)
    : width_(width),
      height_(height),
      pixels_(static_cast<std::size_t>(width) * static_cast<std::size_t>(height)) {
}

Texture Texture::createChecker(int width, int height, int cellSize, Color a, Color b) {
    Texture texture(width, height);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const bool useA = ((x / cellSize) + (y / cellSize)) % 2 == 0;
            texture.setPixel(x, y, useA ? a : b);
        }
    }

    return texture;
}

Texture Texture::loadFromBmp(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return Texture{};
    }

    const std::uint16_t signature = readU16(file);
    if (signature != 0x4D42) {
        return Texture{};
    }

    readU32(file);
    readU16(file);
    readU16(file);
    const std::uint32_t pixelOffset = readU32(file);

    const std::uint32_t headerSize = readU32(file);
    if (headerSize < 40) {
        return Texture{};
    }

    const std::int32_t width = readI32(file);
    const std::int32_t signedHeight = readI32(file);
    const std::uint16_t planes = readU16(file);
    const std::uint16_t bitsPerPixel = readU16(file);
    const std::uint32_t compression = readU32(file);

    if (width <= 0 || signedHeight == 0 || planes != 1 || compression != 0) {
        return Texture{};
    }

    if (bitsPerPixel != 24 && bitsPerPixel != 32) {
        return Texture{};
    }

    const int height = signedHeight < 0 ? -signedHeight : signedHeight;
    const bool topDown = signedHeight < 0;
    Texture texture(width, height);

    const int bytesPerPixel = bitsPerPixel / 8;
    const int rowStride = ((width * bytesPerPixel + 3) / 4) * 4;
    std::vector<std::uint8_t> row(static_cast<std::size_t>(rowStride));

    file.seekg(pixelOffset, std::ios::beg);

    for (int fileY = 0; fileY < height; ++fileY) {
        file.read(reinterpret_cast<char*>(row.data()), rowStride);
        if (!file) {
            return Texture{};
        }

        const int y = topDown ? fileY : height - 1 - fileY;

        for (int x = 0; x < width; ++x) {
            const int source = x * bytesPerPixel;
            const std::uint8_t b = row[static_cast<std::size_t>(source + 0)];
            const std::uint8_t g = row[static_cast<std::size_t>(source + 1)];
            const std::uint8_t r = row[static_cast<std::size_t>(source + 2)];
            const std::uint8_t a = bitsPerPixel == 32 ? row[static_cast<std::size_t>(source + 3)] : 255;
            texture.setPixel(x, y, Color{r, g, b, a});
        }
    }

    return texture;
}

Texture Texture::loadFromPng(const std::string& path) {
    int width = 0;
    int height = 0;
    int channels = 0;

    stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 4);
    if (!data || width <= 0 || height <= 0) {
        stbi_image_free(data);
        return Texture{};
    }

    Texture texture(width, height);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const int source = (y * width + x) * 4;
            texture.setPixel(
                x,
                y,
                Color{
                    data[source + 0],
                    data[source + 1],
                    data[source + 2],
                    data[source + 3]
                }
            );
        }
    }

    stbi_image_free(data);
    return texture;
}

Texture Texture::loadFromFile(const std::string& path) {
    const std::string extension = fileExtension(path);

    if (extension == ".png") {
        return loadFromPng(path);
    }
    if (extension == ".bmp") {
        return loadFromBmp(path);
    }

    return Texture{};
}

Color Texture::pixel(int x, int y) const {
    if (!isValid()) {
        return Color{};
    }

    x = std::max(0, std::min(width_ - 1, x));
    y = std::max(0, std::min(height_ - 1, y));

    return pixels_[static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(x)];
}

void Texture::setPixel(int x, int y, Color color) {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) {
        return;
    }

    pixels_[static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(x)] = color;
}

Camera2D::Camera2D(Vec2 viewportSize)
    : viewportSize_(viewportSize) {
}

Vec2 Camera2D::worldToScreen(Vec2 worldPosition) const {
    return Vec2{worldPosition.x - position_.x, worldPosition.y - position_.y};
}

Vec2 Camera2D::screenToWorld(Vec2 screenPosition) const {
    return Vec2{screenPosition.x + position_.x, screenPosition.y + position_.y};
}

void Camera2D::follow(Vec2 targetCenter) {
    position_.x = targetCenter.x - viewportSize_.x * 0.5f;
    position_.y = targetCenter.y - viewportSize_.y * 0.5f;
}

void Input::setKey(Key key, bool pressed) {
    keys_[static_cast<std::size_t>(key)] = pressed;
}

bool Input::isKeyDown(Key key) const {
    return keys_[static_cast<std::size_t>(key)];
}

Renderer::Renderer(int width, int height)
    : width_(width),
      height_(height) {
    pixels_ = new std::uint32_t[static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_)];
}

Renderer::~Renderer() {
    delete[] pixels_;
    pixels_ = nullptr;
}

void Renderer::clear(Color color) {
    std::fill(
        pixels_,
        pixels_ + static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_),
        pack(color)
    );
}

void Renderer::drawRect(Rect rect, Color color) {
    const int x0 = std::max(0, static_cast<int>(rect.x));
    const int y0 = std::max(0, static_cast<int>(rect.y));
    const int x1 = std::min(width_, static_cast<int>(rect.x + rect.w));
    const int y1 = std::min(height_, static_cast<int>(rect.y + rect.h));

    for (int y = y0; y < y1; ++y) {
        for (int x = x0; x < x1; ++x) {
            putPixel(x, y, color);
        }
    }
}

void Renderer::drawRect(Rect rect, Color color, const Camera2D& camera) {
    const Vec2 screenPosition = camera.worldToScreen(Vec2{rect.x, rect.y});
    drawRect(Rect{screenPosition.x, screenPosition.y, rect.w, rect.h}, color);
}

void Renderer::drawSprite(const Sprite& sprite) {
    const Rect rect{sprite.position.x, sprite.position.y, sprite.size.x, sprite.size.y};
    if (sprite.texture) {
        drawTexture(rect, *sprite.texture);
    } else {
        drawRect(rect, sprite.color);
    }
}

void Renderer::drawSprite(const Sprite& sprite, const Camera2D& camera) {
    const Vec2 screenPosition = camera.worldToScreen(sprite.position);
    Sprite screenSprite = sprite;
    screenSprite.position = screenPosition;
    drawSprite(screenSprite);
}

std::uint32_t Renderer::pack(Color color) const {
    return (static_cast<std::uint32_t>(color.a) << 24)
        | (static_cast<std::uint32_t>(color.r) << 16)
        | (static_cast<std::uint32_t>(color.g) << 8)
        | static_cast<std::uint32_t>(color.b);
}

void Renderer::putPixel(int x, int y, Color color) {
    const std::size_t index = static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(x);

    if (color.a == 255) {
        pixels_[index] = pack(color);
        return;
    }

    if (color.a == 0) {
        return;
    }

    const std::uint32_t destination = pixels_[index];
    const std::uint8_t destinationR = static_cast<std::uint8_t>((destination >> 16) & 0xFF);
    const std::uint8_t destinationG = static_cast<std::uint8_t>((destination >> 8) & 0xFF);
    const std::uint8_t destinationB = static_cast<std::uint8_t>(destination & 0xFF);

    const int alpha = color.a;
    const int inverseAlpha = 255 - alpha;

    const Color blended{
        static_cast<std::uint8_t>((color.r * alpha + destinationR * inverseAlpha) / 255),
        static_cast<std::uint8_t>((color.g * alpha + destinationG * inverseAlpha) / 255),
        static_cast<std::uint8_t>((color.b * alpha + destinationB * inverseAlpha) / 255),
        255
    };

    pixels_[index] = pack(blended);
}

void Renderer::drawTexture(Rect rect, const Texture& texture) {
    if (rect.w <= 0.0f || rect.h <= 0.0f || texture.width() <= 0 || texture.height() <= 0) {
        return;
    }

    const int x0 = std::max(0, static_cast<int>(rect.x));
    const int y0 = std::max(0, static_cast<int>(rect.y));
    const int x1 = std::min(width_, static_cast<int>(rect.x + rect.w));
    const int y1 = std::min(height_, static_cast<int>(rect.y + rect.h));

    for (int y = y0; y < y1; ++y) {
        for (int x = x0; x < x1; ++x) {
            const float u = (static_cast<float>(x) - rect.x) / rect.w;
            const float v = (static_cast<float>(y) - rect.y) / rect.h;
            const int sourceX = std::max(0, std::min(texture.width() - 1, static_cast<int>(u * static_cast<float>(texture.width()))));
            const int sourceY = std::max(0, std::min(texture.height() - 1, static_cast<int>(v * static_cast<float>(texture.height()))));
            putPixel(x, y, texture.pixel(sourceX, sourceY));
        }
    }
}

void SpriteBatch::begin(const Camera2D& camera) {
    camera_ = &camera;
    sprites_.clear();
}

void SpriteBatch::draw(const Sprite& sprite) {
    sprites_.push_back(sprite);
}

void SpriteBatch::flush(Renderer& renderer) {
    if (!camera_) {
        return;
    }

    for (const Sprite& sprite : sprites_) {
        renderer.drawSprite(sprite, *camera_);
    }

    sprites_.clear();
}

void SpriteBatch::clear() {
    sprites_.clear();
    camera_ = nullptr;
}

} // namespace tiny2d
