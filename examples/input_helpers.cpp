#include <cppwindow/cppwindow.hpp>

#include <array>
#include <glad/glad.h>
#include <iomanip>
#include <iostream>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string_view>

#include "status_text.hpp"

using namespace cwin;

namespace {

void drawInputPanel(
    uint32_t framebufferWidth,
    uint32_t framebufferHeight,
    std::span<const std::string_view> controls,
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
    const int scale = (framebufferWidth < 1100 || framebufferHeight < 650) ? 1 : 2;
    const int lineHeight = 7 * scale + 6;
    const int panelHeight = lineHeight * (static_cast<int>(controls.size()) + 2) + 18;

    example::drawRect(0, 0, width, panelHeight, 0.02f, 0.025f, 0.03f);
    example::drawRect(0, panelHeight - 2, width, 2, 0.14f, 0.45f, 0.70f);

    int y = panelHeight - lineHeight;
    for (std::string_view control : controls) {
        example::drawText(control, 12, y, scale);
        y -= lineHeight;
    }
    example::drawText(state, 12, y, scale);
    y -= lineHeight;
    example::drawText(mouse, 12, y, scale);
}

}  // namespace

int main()
{
    auto& ctx = WindowContext::get();

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
    const ActionId quit = actions.getOrCreateActionId("quit");
    const ActionId jump = actions.getOrCreateActionId("jump");
    const ActionId moveX = actions.getOrCreateActionId("move_x");
    const ActionId fire = actions.getOrCreateActionId("fire");
    const ActionId save = actions.getOrCreateActionId("save");
    const ActionId leftShiftA = actions.getOrCreateActionId("left_shift_a");
    const ActionId rightShiftA = actions.getOrCreateActionId("right_shift_a");
    const ActionId toggleGameplay = actions.getOrCreateActionId("toggle_gameplay");
    const ActionId capture = actions.getOrCreateActionId("capture");
    const ActionId centerMouse = actions.getOrCreateActionId("center_mouse");

    actions.setMetadata(jump, { "Jump", "Keyboard Space or gamepad A" })
        .setMetadata(moveX, { "Move X", "Gamepad left stick horizontal axis" })
        .setMetadata(fire, { "Fire", "Left mouse button" })
        .bindKey(quit, Key::Escape)
        .bindKey(jump, Key::Space)
        .bindGamepadButton(jump, GamepadButton::A)
        .bindGamepadAxis(moveX, GamepadAxis::LeftX, 0.20f)
        .bindMouseButton(fire, MouseButton::Left)
        .bindKey(save, Key::S, Modifiers{ .control = true })
        .bindKeyCombo(leftShiftA, Key::A, { Key::LShift })
        .bindKeyCombo(rightShiftA, Key::A, { Key::RShift })
        .bindKey(toggleGameplay, Key::G)
        .bindKey(capture, Key::C)
        .bindKey(centerMouse, Key::R)
        .setContext(jump, "gameplay")
        .setContext(fire, "gameplay")
        .setContext(moveX, "gameplay");

    bool captured = false;
    bool rawMouse = false;
    bool gameplayEnabled = true;
    int saveCount = 0;

    std::cout << "Input Helpers controls:\n";
    std::cout << "  Escape: quit\n";
    std::cout << "  Space: jump action\n";
    std::cout << "  Gamepad A: jump action\n";
    std::cout << "  Left stick X: move_x axis action\n";
    std::cout << "  Left mouse button: fire action\n";
    std::cout << "  Ctrl+S: save action\n";
    std::cout << "  Left Shift+A: left_shift_a action\n";
    std::cout << "  Right Shift+A: right_shift_a action\n";
    std::cout << "  G: toggle gameplay action context\n";
    std::cout << "  C: toggle captured cursor and raw mouse motion\n";
    std::cout << "  R: center mouse cursor\n";

    static constexpr std::array<std::string_view, 11> ControlLines{
        "ESC: QUIT",          "SPACE: JUMP",    "GAMEPAD A: JUMP",    "LEFT STICK X: MOVE",
        "LMB: FIRE",          "CTRL+S: SAVE",   "LEFT SHIFT+A: LS+A", "RIGHT SHIFT+A: RS+A",
        "G: TOGGLE GAMEPLAY", "C: CAPTURE RAW", "R: CENTER MOUSE",
    };
    EventDispatcher dispatcher;
    dispatcher.on<Event::Closed>([&] {
        window.requestClose();
    });

    while (!window.shouldClose()) {
        ctx.pollEvents();
        dispatcher.dispatch(window.events());

        actions.update(window.getInput(), ctx.getGamepadState(0));

        if (actions.isPressed(quit)) {
            window.requestClose();
        }

        if (actions.isPressed(save)) {
            ++saveCount;
        }

        if (actions.isPressed(toggleGameplay)) {
            gameplayEnabled = !gameplayEnabled;
            actions.setContextEnabled("gameplay", gameplayEnabled);
        }

        if (actions.isPressed(capture)) {
            captured = !captured;
            window.setCursorMode(captured ? CursorMode::Captured : CursorMode::Normal);
            rawMouse = captured && ctx.isRawMouseMotionSupported();
            if (!window.setRawMouseMotion(rawMouse)) {
                rawMouse = false;
            }
        }

        if (actions.isPressed(centerMouse)) {
            auto [width, height] = window.getSize();
            window.setMousePosition(width * 0.5, height * 0.5);
        }

        auto [x, y] = window.getInput().getMousePosition();
        auto [dx, dy] = window.getInput().getMouseDelta();
        const DpiScale dpi = window.getDpiScale();
        auto [fbMouseX, fbMouseY] = dpi.windowToFramebuffer(x, y);

        std::ostringstream state;
        state << "JUMP:" << (actions.isDown(jump) ? "ON" : "OFF")
              << " FIRE:" << (actions.isDown(fire) ? "ON" : "OFF")
              << " LS+A:" << (actions.isDown(leftShiftA) ? "ON" : "OFF")
              << " RS+A:" << (actions.isDown(rightShiftA) ? "ON" : "OFF")
              << " GAME:" << (gameplayEnabled ? "ON" : "OFF") << " SAVE:" << saveCount
              << " CAP:" << (captured ? "ON" : "OFF") << " RAW:" << (rawMouse ? "ON" : "OFF");

        std::ostringstream mouse;
        mouse << std::fixed << std::setprecision(2) << "MOVE:" << actions.getAxis(moveX)
              << " MOUSE:" << static_cast<int>(x) << "," << static_cast<int>(y)
              << " FB:" << static_cast<int>(fbMouseX) << "," << static_cast<int>(fbMouseY)
              << " DELTA:" << static_cast<int>(dx) << "," << static_cast<int>(dy);

        auto [fbWidth, fbHeight] = window.getFramebufferSize();
        drawInputPanel(
            fbWidth,
            fbHeight,
            std::span<const std::string_view>{ ControlLines.data(), ControlLines.size() },
            state.str(),
            mouse.str());
        window.swapBuffers();

        frameLimiter.wait();
    }
}
