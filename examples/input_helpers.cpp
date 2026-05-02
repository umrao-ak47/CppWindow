#include <cppwindow/cppwindow.hpp>

#include <glad/glad.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "status_text.hpp"

using namespace cwin;

namespace {

void drawInputPanel(
    uint32_t framebufferWidth,
    uint32_t framebufferHeight,
    const std::string& controls,
    const std::string& state,
    const std::string& mouse)
{
    glViewport(
        0,
        0,
        static_cast<GLsizei>(framebufferWidth),
        static_cast<GLsizei>(framebufferHeight));
    glClearColor(0.045f, 0.055f, 0.065f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    const int width = static_cast<int>(framebufferWidth);
    const int scale = framebufferWidth < 900 ? 1 : 2;
    const int lineHeight = 7 * scale + 10;
    const int panelHeight = lineHeight * 3 + 18;

    example::drawRect(0, 0, width, panelHeight, 0.02f, 0.025f, 0.03f);
    example::drawRect(0, panelHeight - 2, width, 2, 0.14f, 0.45f, 0.70f);
    example::drawText(controls, 12, panelHeight - lineHeight, scale);
    example::drawText(state, 12, panelHeight - lineHeight * 2, scale);
    example::drawText(mouse, 12, panelHeight - lineHeight * 3, scale);
}

}  // namespace

int main()
{
    auto& ctx = WindowContext::Get();

    auto window = WindowBuilder{}
                      .title("Input Helpers")
                      .size(900, 480)
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

    ActionMap actions;
    actions.bindKey("quit", Key::Escape)
        .bindKey("jump", Key::Space)
        .bindGamepadButton("jump", GamepadButton::A)
        .bindMouseButton("fire", MouseButton::Left)
        .bindKey("capture", Key::C)
        .bindKey("center_mouse", Key::R);

    bool captured = false;
    bool rawMouse = false;

    std::cout << "Input Helpers controls:\n";
    std::cout << "  Escape: quit\n";
    std::cout << "  Space or gamepad A: jump action\n";
    std::cout << "  Left mouse button: fire action\n";
    std::cout << "  C: toggle captured cursor and raw mouse motion\n";
    std::cout << "  R: center mouse cursor\n";

    while (!window.shouldClose()) {
        ctx.pollEvents();

        for (const auto& event : window.events()) {
            if (event.is<Event::Closed>()) {
                window.requestClose();
            }
        }

        actions.update(window.getInput(), ctx.getGamepadState(0));

        if (actions.isPressed("quit")) {
            window.requestClose();
        }

        if (actions.isPressed("capture")) {
            captured = !captured;
            window.setCursorMode(captured ? CursorMode::Captured : CursorMode::Normal);
            rawMouse = captured && ctx.isRawMouseMotionSupported();
            if (!window.setRawMouseMotion(rawMouse)) {
                rawMouse = false;
            }
        }

        if (actions.isPressed("center_mouse")) {
            auto [width, height] = window.getSize();
            window.setMousePosition(width * 0.5, height * 0.5);
        }

        auto [x, y] = window.getInput().getMousePosition();
        auto [dx, dy] = window.getInput().getMouseDelta();
        const DpiScale dpi = window.getDpiScale();
        auto [fbMouseX, fbMouseY] = dpi.windowToFramebuffer(x, y);

        std::ostringstream state;
        state << "JUMP:" << (actions.isDown("jump") ? "ON" : "OFF")
              << " FIRE:" << (actions.isDown("fire") ? "ON" : "OFF")
              << " CAP:" << (captured ? "ON" : "OFF") << " RAW:" << (rawMouse ? "ON" : "OFF");

        std::ostringstream mouse;
        mouse << "MOUSE:" << static_cast<int>(x) << "," << static_cast<int>(y)
              << " FB:" << static_cast<int>(fbMouseX) << "," << static_cast<int>(fbMouseY)
              << " DELTA:" << static_cast<int>(dx) << "," << static_cast<int>(dy);

        auto [fbWidth, fbHeight] = window.getFrameBufferSize();
        drawInputPanel(
            fbWidth,
            fbHeight,
            "ESC QUIT  SPACE/A JUMP  LMB FIRE  C CAPTURE  R CENTER",
            state.str(),
            mouse.str());
        window.swapBuffers();

        frameLimiter.wait();
    }
}
