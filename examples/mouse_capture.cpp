#include <cppwindow/cppwindow.hpp>

#include <glad/glad.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "common/opengl_status_text.hpp"

using namespace cwin;

namespace {

std::string makeStatus(double deltaX, double deltaY, bool inside, bool captured)
{
    std::ostringstream status;
    status << std::fixed << std::setprecision(1) << "DX:" << std::showpos << deltaX
           << " DY:" << deltaY << std::noshowpos << " IN:" << (inside ? "YES" : "NO")
           << " CAP:" << (captured ? "ON" : "OFF");
    return status.str();
}

}  // namespace

int main()
{
    auto& ctx = WindowContext::get();

    auto window = WindowBuilder{}
                      .title("Mouse Capture")
                      .size(960, 540)
                      .openGL({ 4, 1, true })
                      .resizable()
                      .build();
    window.makeContextCurrent();

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(ctx.procLoader()))) {
        throw std::runtime_error("Failed to initialize GLAD");
    }
    const bool useVSync = true;
    window.setVSync(useVSync);
    FrameLimiter frameLimiter(120.0);
    frameLimiter.setVSyncEnabled(useVSync);

    std::cout << "Controls:\n"
              << "  C: toggle captured cursor\n"
              << "  Esc: close\n";

    bool captured = false;
    double displayDeltaX = 0.0;
    double displayDeltaY = 0.0;
    Clock statusResetClock;
    EventDispatcher dispatcher;
    dispatcher
        .on<Event::Closed>([&] {
            window.requestClose();
        })
        .on<Event::KeyPressed>([&](const Event::KeyPressed& key) {
            if (key.key == Key::Escape) {
                window.requestClose();
            } else if (key.key == Key::C) {
                captured = !captured;
                window.setCursorMode(captured ? CursorMode::Captured : CursorMode::Normal);
                std::cout << "captured: " << captured << "\n";
            }
        })
        .on<Event::MouseEntered>([&] {
            std::cout << "mouse entered\n";
        })
        .on<Event::MouseLeft>([&] {
            std::cout << "mouse left\n";
        });

    while (!window.shouldClose()) {
        ctx.pollEvents();
        dispatcher.dispatch(window.events());

        auto [deltaX, deltaY] = window.input().mouseDelta();
        displayDeltaX += deltaX;
        displayDeltaY += deltaY;

        auto [fbWidth, fbHeight] = window.framebufferSize();
        const std::string status =
            makeStatus(displayDeltaX, displayDeltaY, window.input().isMouseInside(), captured);
        example::drawStatusBar(fbWidth, fbHeight, status);
        window.swapBuffers();

        if (statusResetClock.elapsedSeconds() >= 0.08) {
            displayDeltaX = 0.0;
            displayDeltaY = 0.0;
            statusResetClock.reset();
        }

        frameLimiter.wait();
    }
}
