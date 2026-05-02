#include "Engine.h"

#include <algorithm>
#include <chrono>
#include <stdexcept>

namespace tiny2d {
namespace {

constexpr const wchar_t* WindowClassName = L"Tiny2DWindowClass";

float secondsSince(std::chrono::steady_clock::time_point& previous) {
    const auto now = std::chrono::steady_clock::now();
    const std::chrono::duration<float> delta = now - previous;
    previous = now;
    return delta.count();
}

} // namespace

void Input::setKey(int virtualKey, bool pressed) {
    if (virtualKey >= 0 && virtualKey < static_cast<int>(keys_.size())) {
        keys_[static_cast<std::size_t>(virtualKey)] = pressed;
    }
}

bool Input::isKeyDown(int virtualKey) const {
    if (virtualKey < 0 || virtualKey >= static_cast<int>(keys_.size())) {
        return false;
    }

    return keys_[static_cast<std::size_t>(virtualKey)];
}

Renderer::Renderer(int width, int height)
    : width_(width),
      height_(height) {
    pixels_ = new std::uint32_t[static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_)];

    bitmapInfo_.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo_.bmiHeader.biWidth = width_;
    bitmapInfo_.bmiHeader.biHeight = -height_;
    bitmapInfo_.bmiHeader.biPlanes = 1;
    bitmapInfo_.bmiHeader.biBitCount = 32;
    bitmapInfo_.bmiHeader.biCompression = BI_RGB;
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

void Renderer::drawSprite(const Sprite& sprite) {
    drawRect(Rect{sprite.position.x, sprite.position.y, sprite.size.x, sprite.size.y}, sprite.color);
}

void Renderer::present(HDC deviceContext) {
    StretchDIBits(
        deviceContext,
        0,
        0,
        width_,
        height_,
        0,
        0,
        width_,
        height_,
        pixels_,
        &bitmapInfo_,
        DIB_RGB_COLORS,
        SRCCOPY
    );
}

std::uint32_t Renderer::pack(Color color) const {
    return (static_cast<std::uint32_t>(color.a) << 24)
        | (static_cast<std::uint32_t>(color.r) << 16)
        | (static_cast<std::uint32_t>(color.g) << 8)
        | static_cast<std::uint32_t>(color.b);
}

void Renderer::putPixel(int x, int y, Color color) {
    pixels_[static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(x)] = pack(color);
}

Application::Application(int width, int height, const wchar_t* title)
    : width_(width),
      height_(height),
      renderer_(width, height) {
    WNDCLASSW windowClass{};
    windowClass.lpfnWndProc = Application::windowProc;
    windowClass.hInstance = GetModuleHandleW(nullptr);
    windowClass.lpszClassName = WindowClassName;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);

    RegisterClassW(&windowClass);

    const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT clientRect{0, 0, width_, height_};
    AdjustWindowRect(&clientRect, style, FALSE);

    window_ = CreateWindowExW(
        0,
        WindowClassName,
        title,
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        clientRect.right - clientRect.left,
        clientRect.bottom - clientRect.top,
        nullptr,
        nullptr,
        GetModuleHandleW(nullptr),
        this
    );

    if (!window_) {
        throw std::runtime_error("Could not create Win32 window.");
    }

    ShowWindow(window_, SW_SHOW);
}

int Application::run() {
    auto previousTime = std::chrono::steady_clock::now();

    while (running_) {
        MSG message{};
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        const float deltaSeconds = secondsSince(previousTime);

        game_.update(deltaSeconds, input_, renderer_);
        renderer_.clear(Color::rgb(24, 26, 32));
        game_.render(renderer_);

        HDC deviceContext = GetDC(window_);
        renderer_.present(deviceContext);
        ReleaseDC(window_, deviceContext);
    }

    return 0;
}

LRESULT CALLBACK Application::windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    Application* application = nullptr;

    if (message == WM_NCCREATE) {
        const auto* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
        application = static_cast<Application*>(createStruct->lpCreateParams);
        application->window_ = window;
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(application));
    } else {
        application = reinterpret_cast<Application*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    }

    if (application) {
        return application->handleMessage(message, wParam, lParam);
    }

    return DefWindowProcW(window, message, wParam, lParam);
}

LRESULT Application::handleMessage(UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CLOSE:
    case WM_DESTROY:
        running_ = false;
        PostQuitMessage(0);
        return 0;

    case WM_KEYDOWN:
        input_.setKey(static_cast<int>(wParam), true);
        return 0;

    case WM_KEYUP:
        input_.setKey(static_cast<int>(wParam), false);
        return 0;

    default:
        return DefWindowProcW(window_, message, wParam, lParam);
    }
}

} // namespace tiny2d
