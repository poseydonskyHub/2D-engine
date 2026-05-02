#include "Engine.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <chrono>
#include <stdexcept>

namespace tiny2d {
namespace {

struct LinuxState {
    Display* display = nullptr;
    Window window = 0;
    GC graphicsContext = 0;
    Atom closeMessage = 0;
};

float secondsSince(std::chrono::steady_clock::time_point& previous) {
    const auto now = std::chrono::steady_clock::now();
    const std::chrono::duration<float> delta = now - previous;
    previous = now;
    return delta.count();
}

LinuxState* stateFrom(Application& app) {
    return static_cast<LinuxState*>(app.platformData());
}

Input::Key translateKey(KeySym key) {
    switch (key) {
    case XK_a:
    case XK_A:
        return Input::A;
    case XK_d:
    case XK_D:
        return Input::D;
    case XK_w:
    case XK_W:
        return Input::W;
    case XK_s:
    case XK_S:
        return Input::S;
    case XK_Left:
        return Input::Left;
    case XK_Right:
        return Input::Right;
    case XK_Up:
        return Input::Up;
    case XK_Down:
        return Input::Down;
    default:
        return Input::Count;
    }
}

void setTranslatedKey(Input& input, XKeyEvent& event, bool pressed) {
    const Input::Key translated = translateKey(XLookupKeysym(&event, 0));
    if (translated != Input::Count) {
        input.setKey(translated, pressed);
    }
}

void present(LinuxState& state, Renderer& renderer) {
    XImage* image = XCreateImage(
        state.display,
        DefaultVisual(state.display, DefaultScreen(state.display)),
        24,
        ZPixmap,
        0,
        reinterpret_cast<char*>(const_cast<std::uint32_t*>(renderer.pixels())),
        static_cast<unsigned int>(renderer.width()),
        static_cast<unsigned int>(renderer.height()),
        32,
        0
    );

    if (!image) {
        return;
    }

    XPutImage(
        state.display,
        state.window,
        state.graphicsContext,
        image,
        0,
        0,
        0,
        0,
        static_cast<unsigned int>(renderer.width()),
        static_cast<unsigned int>(renderer.height())
    );
    XFlush(state.display);

    image->data = nullptr;
    XDestroyImage(image);
}

} // namespace

Application::Application(int width, int height, const char* title)
    : width_(width),
      height_(height),
      platform_(new LinuxState{}),
      renderer_(width, height) {
    auto* state = stateFrom(*this);

    state->display = XOpenDisplay(nullptr);
    if (!state->display) {
        throw std::runtime_error("Could not open X11 display.");
    }

    const int screen = DefaultScreen(state->display);
    state->window = XCreateSimpleWindow(
        state->display,
        RootWindow(state->display, screen),
        0,
        0,
        static_cast<unsigned int>(width_),
        static_cast<unsigned int>(height_),
        0,
        BlackPixel(state->display, screen),
        BlackPixel(state->display, screen)
    );

    XStoreName(state->display, state->window, title);
    XSelectInput(state->display, state->window, ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask);

    state->closeMessage = XInternAtom(state->display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(state->display, state->window, &state->closeMessage, 1);

    state->graphicsContext = XCreateGC(state->display, state->window, 0, nullptr);
    XMapWindow(state->display, state->window);
}

Application::~Application() {
    auto* state = stateFrom(*this);
    if (state) {
        if (state->graphicsContext) {
            XFreeGC(state->display, state->graphicsContext);
        }
        if (state->window) {
            XDestroyWindow(state->display, state->window);
        }
        if (state->display) {
            XCloseDisplay(state->display);
        }
    }
    delete state;
    platform_ = nullptr;
}

int Application::run() {
    auto* state = stateFrom(*this);
    auto previousTime = std::chrono::steady_clock::now();

    while (isRunning()) {
        while (XPending(state->display) > 0) {
            XEvent event{};
            XNextEvent(state->display, &event);

            if (event.type == KeyPress) {
                setTranslatedKey(input(), event.xkey, true);
            } else if (event.type == KeyRelease) {
                setTranslatedKey(input(), event.xkey, false);
            } else if (event.type == ClientMessage && static_cast<Atom>(event.xclient.data.l[0]) == state->closeMessage) {
                stop();
            } else if (event.type == DestroyNotify) {
                stop();
            }
        }

        const float deltaSeconds = secondsSince(previousTime);

        game().update(deltaSeconds, input(), renderer());
        renderer().clear(Color::rgb(24, 26, 32));
        game().render(renderer());
        present(*state, renderer());
    }

    return 0;
}

} // namespace tiny2d
