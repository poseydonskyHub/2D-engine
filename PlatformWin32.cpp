#include "Engine.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <chrono>
#include <stdexcept>

namespace tiny2d {
namespace {

constexpr const char* WindowClassName = "Tiny2DWindowClass";

struct Win32State {
    HWND window = nullptr;
    BITMAPINFO bitmapInfo{};
};

float secondsSince(std::chrono::steady_clock::time_point& previous) {
    const auto now = std::chrono::steady_clock::now();
    const std::chrono::duration<float> delta = now - previous;
    previous = now;
    return delta.count();
}

Win32State* stateFrom(Application& app) {
    return static_cast<Win32State*>(app.platformData());
}

Input::Key translateKey(WPARAM key) {
    switch (key) {
    case 'A':
        return Input::A;
    case 'D':
        return Input::D;
    case 'W':
        return Input::W;
    case 'S':
        return Input::S;
    case VK_LEFT:
        return Input::Left;
    case VK_RIGHT:
        return Input::Right;
    case VK_UP:
        return Input::Up;
    case VK_DOWN:
        return Input::Down;
    default:
        return Input::Count;
    }
}

void setTranslatedKey(Input& input, WPARAM key, bool pressed) {
    const Input::Key translated = translateKey(key);
    if (translated != Input::Count) {
        input.setKey(translated, pressed);
    }
}

void present(HWND window, Renderer& renderer, BITMAPINFO& bitmapInfo) {
    HDC deviceContext = GetDC(window);
    StretchDIBits(
        deviceContext,
        0,
        0,
        renderer.width(),
        renderer.height(),
        0,
        0,
        renderer.width(),
        renderer.height(),
        renderer.pixels(),
        &bitmapInfo,
        DIB_RGB_COLORS,
        SRCCOPY
    );
    ReleaseDC(window, deviceContext);
}

LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    Application* application = nullptr;

    if (message == WM_NCCREATE) {
        const auto* createStruct = reinterpret_cast<CREATESTRUCTA*>(lParam);
        application = static_cast<Application*>(createStruct->lpCreateParams);
        stateFrom(*application)->window = window;
        SetWindowLongPtrA(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(application));
    } else {
        application = reinterpret_cast<Application*>(GetWindowLongPtrA(window, GWLP_USERDATA));
    }

    if (!application) {
        return DefWindowProcA(window, message, wParam, lParam);
    }

    switch (message) {
    case WM_CLOSE:
    case WM_DESTROY:
        application->stop();
        PostQuitMessage(0);
        return 0;

    case WM_KEYDOWN:
        setTranslatedKey(application->input(), wParam, true);
        return 0;

    case WM_KEYUP:
        setTranslatedKey(application->input(), wParam, false);
        return 0;

    default:
        return DefWindowProcA(window, message, wParam, lParam);
    }
}

} // namespace

Application::Application(int width, int height, const char* title)
    : width_(width),
      height_(height),
      platform_(new Win32State{}),
      renderer_(width, height) {
    auto* state = stateFrom(*this);

    state->bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    state->bitmapInfo.bmiHeader.biWidth = width_;
    state->bitmapInfo.bmiHeader.biHeight = -height_;
    state->bitmapInfo.bmiHeader.biPlanes = 1;
    state->bitmapInfo.bmiHeader.biBitCount = 32;
    state->bitmapInfo.bmiHeader.biCompression = BI_RGB;

    WNDCLASSA windowClass{};
    windowClass.lpfnWndProc = windowProc;
    windowClass.hInstance = GetModuleHandleA(nullptr);
    windowClass.lpszClassName = WindowClassName;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);

    RegisterClassA(&windowClass);

    const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT clientRect{0, 0, width_, height_};
    AdjustWindowRect(&clientRect, style, FALSE);

    state->window = CreateWindowExA(
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
        GetModuleHandleA(nullptr),
        this
    );

    if (!state->window) {
        throw std::runtime_error("Could not create Win32 window.");
    }

    ShowWindow(state->window, SW_SHOW);
}

Application::~Application() {
    auto* state = stateFrom(*this);
    if (state && state->window) {
        DestroyWindow(state->window);
    }
    delete state;
    platform_ = nullptr;
}

int Application::run() {
    auto* state = stateFrom(*this);
    auto previousTime = std::chrono::steady_clock::now();

    while (isRunning()) {
        MSG message{};
        while (PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }

        const float deltaSeconds = secondsSince(previousTime);

        game().update(deltaSeconds, input(), renderer());
        renderer().clear(Color::rgb(24, 26, 32));
        game().render(renderer());
        present(state->window, renderer(), state->bitmapInfo);
    }

    return 0;
}

} // namespace tiny2d
