#include <cppwindow/cppwindow.hpp>

#include <glad/glad.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "status_text.hpp"

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
    auto& ctx = WindowContext::Get();

    auto window = WindowBuilder{}
                      .title("Mouse Capture")
                      .size(960, 540)
                      .openGL({ 4, 1, true })
                      .resizable()
                      .build();
    window.makeContextCurrent();

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(ctx.getProcLoader()))) {
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

    while (!window.shouldClose()) {
        ctx.pollEvents();

        for (const auto& event : window.events()) {
            if (event.is<Event::Closed>()) {
                window.requestClose();
            }

            if (const auto* key = event.getIf<Event::KeyPressed>()) {
                if (key->key == Key::Escape) {
                    window.requestClose();
                } else if (key->key == Key::C) {
                    captured = !captured;
                    window.setCursorMode(captured ? CursorMode::Captured : CursorMode::Normal);
                    std::cout << "captured: " << captured << "\n";
                }
            }

            if (event.is<Event::MouseEntered>()) {
                std::cout << "mouse entered\n";
            } else if (event.is<Event::MouseLeft>()) {
                std::cout << "mouse left\n";
            }
        }

        auto [deltaX, deltaY] = window.getInput().getMouseDelta();
        displayDeltaX += deltaX;
        displayDeltaY += deltaY;

        auto [fbWidth, fbHeight] = window.getFrameBufferSize();
        const std::string status =
            makeStatus(displayDeltaX, displayDeltaY, window.getInput().isMouseInside(), captured);
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
