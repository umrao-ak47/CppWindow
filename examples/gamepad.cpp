#include <cppwindow/cppwindow.hpp>

#include <chrono>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

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
    auto& ctx = WindowContext::Get();

    auto window = WindowBuilder{}.title("Gamepad").size(720, 360).noAPI().resizable().build();

    std::cout << "Connect a standard-mapped gamepad. Escape closes.\n";

    std::string lastTitle;

    while (!window.shouldClose()) {
        ctx.pollEvents();

        for (const auto& event : window.events()) {
            if (event.is<Event::Closed>()) {
                window.requestClose();
            }

            if (const auto* key = event.getIf<Event::KeyPressed>()) {
                if (key->key == Key::Escape) {
                    window.requestClose();
                }
            }

            if (const auto* connected = event.getIf<Event::GamepadConnected>()) {
                std::cout << "gamepad connected [" << connected->gamepadId << "] "
                          << connected->name << "\n";
            }

            if (const auto* disconnected = event.getIf<Event::GamepadDisconnected>()) {
                std::cout << "gamepad disconnected [" << disconnected->gamepadId << "]\n";
            }

            if (const auto* button = event.getIf<Event::GamepadButtonPressed>()) {
                std::cout << "button pressed [" << button->gamepadId << "] "
                          << buttonName(button->button) << "\n";
            }

            if (const auto* button = event.getIf<Event::GamepadButtonReleased>()) {
                std::cout << "button released [" << button->gamepadId << "] "
                          << buttonName(button->button) << "\n";
            }

            if (const auto* axis = event.getIf<Event::GamepadAxisMoved>()) {
                if (std::abs(axis->value) > 0.5f) {
                    std::cout << "axis [" << axis->gamepadId << "] " << axisName(axis->axis)
                              << " = " << axis->value << "\n";
                }
            }
        }

        std::ostringstream title;
        title << "Gamepad";

        const auto gamepads = ctx.getGamepads();
        if (gamepads.empty()) {
            title << " - no device";
        } else if (const auto state = ctx.getGamepadState(gamepads.front().id)) {
            title << " - " << state->name << " LX " << state->getAxis(GamepadAxis::LeftX)
                  << " LY " << state->getAxis(GamepadAxis::LeftY)
                  << " A " << state->isButtonDown(GamepadButton::A);
        } else {
            title << " - " << gamepads.front().name << " (no standard mapping)";
        }

        const std::string nextTitle = title.str();
        if (nextTitle != lastTitle) {
            window.setTitle(nextTitle);
            lastTitle = nextTitle;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}
