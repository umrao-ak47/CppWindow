#include <cppwindow/cppwindow.hpp>

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

using namespace cwin;

namespace {

const char* buttonName(GamepadButton button)
{
    switch (button) {
        case GamepadButton::A:
            return "A";
        case GamepadButton::B:
            return "B";
        case GamepadButton::X:
            return "X";
        case GamepadButton::Y:
            return "Y";
        case GamepadButton::LeftBumper:
            return "left bumper";
        case GamepadButton::RightBumper:
            return "right bumper";
        case GamepadButton::Back:
            return "back";
        case GamepadButton::Start:
            return "start";
        case GamepadButton::Guide:
            return "guide";
        case GamepadButton::LeftThumb:
            return "left thumb";
        case GamepadButton::RightThumb:
            return "right thumb";
        case GamepadButton::DPadUp:
            return "dpad up";
        case GamepadButton::DPadRight:
            return "dpad right";
        case GamepadButton::DPadDown:
            return "dpad down";
        case GamepadButton::DPadLeft:
            return "dpad left";
    }

    return "unknown";
}

const char* axisName(GamepadAxis axis)
{
    switch (axis) {
        case GamepadAxis::LeftX:
            return "left x";
        case GamepadAxis::LeftY:
            return "left y";
        case GamepadAxis::RightX:
            return "right x";
        case GamepadAxis::RightY:
            return "right y";
        case GamepadAxis::LeftTrigger:
            return "left trigger";
        case GamepadAxis::RightTrigger:
            return "right trigger";
    }

    return "unknown";
}

}  // namespace

int main()
{
    auto& ctx = WindowContext::get();

    auto window =
        WindowBuilder{}.title("Gamepad").size(720, 360).noGraphicsApi().resizable().build();

    std::cout << "Controls:\n"
              << "  Connect standard gamepad: show state\n"
              << "  Esc: close\n";

    std::string lastTitle;
    FrameLimiter frameLimiter(60.0);
    EventDispatcher dispatcher;
    dispatcher
        .on<Event::Closed>([&] {
            window.requestClose();
        })
        .on<Event::KeyPressed>([&](const Event::KeyPressed& key) {
            if (key.key == Key::Escape) {
                window.requestClose();
            }
        })
        .on<Event::GamepadConnected>([](const Event::GamepadConnected& connected) {
            std::cout << "gamepad connected [" << connected.gamepadId << "] " << connected.name
                      << "\n";
        })
        .on<Event::GamepadDisconnected>([](const Event::GamepadDisconnected& disconnected) {
            std::cout << "gamepad disconnected [" << disconnected.gamepadId << "]\n";
        })
        .on<Event::GamepadButtonPressed>([](const Event::GamepadButtonPressed& button) {
            std::cout << "button pressed [" << button.gamepadId << "] " << buttonName(button.button)
                      << "\n";
        })
        .on<Event::GamepadButtonReleased>([](const Event::GamepadButtonReleased& button) {
            std::cout << "button released [" << button.gamepadId << "] "
                      << buttonName(button.button) << "\n";
        })
        .on<Event::GamepadAxisMoved>([](const Event::GamepadAxisMoved& axis) {
            if (std::abs(axis.value) > 0.5f) {
                std::cout << "axis [" << axis.gamepadId << "] " << axisName(axis.axis) << " = "
                          << axis.value << "\n";
            }
        });

    while (!window.shouldClose()) {
        ctx.pollEvents();
        dispatcher.dispatch(window.events());

        std::ostringstream title;
        title << "Gamepad";

        const auto gamepads = ctx.gamepads();
        if (gamepads.empty()) {
            title << " - no device";
        } else if (const auto state = ctx.gamepadState(gamepads.front().id)) {
            title << " - " << state->name << " LX " << state->axis(GamepadAxis::LeftX) << " LY "
                  << state->axis(GamepadAxis::LeftY) << " A "
                  << state->isButtonDown(GamepadButton::A);
        } else {
            title << " - " << gamepads.front().name << " (no standard mapping)";
        }

        const std::string nextTitle = title.str();
        if (nextTitle != lastTitle) {
            window.setTitle(nextTitle);
            lastTitle = nextTitle;
        }

        frameLimiter.wait();
    }
}
