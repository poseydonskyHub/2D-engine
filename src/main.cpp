#include "Engine.h"

#include <exception>

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    try {
        tiny2d::Application app(800, 480, L"2D Engine");
        return app.run();
    } catch (const std::exception&) {
        MessageBoxW(nullptr, L"Engine startup failed.", L"Tiny2D Error", MB_OK | MB_ICONERROR);
        return 1;
    }
}

