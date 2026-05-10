/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#ifndef CPPWINDOW_HEADER_GLFW_GAMEPAD_HPP
#define CPPWINDOW_HEADER_GLFW_GAMEPAD_HPP

#include <cppwindow/events.hpp>
#include <cppwindow/input.hpp>

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace cwin::backend::glfw {

class DeviceEventPoller
{
public:
    DeviceEventPoller();

    void poll(std::vector<Event>& events);

private:
    struct JoystickSnapshot
    {
        bool present = false;
        std::string name;
        bool standardMapping = false;
        std::vector<unsigned char> buttons;
        std::vector<float> axes;

        void reset() noexcept;
        void reserveStorage();
    };

    static void readJoystickSnapshot(uint32_t joystickId, JoystickSnapshot& snapshot);

    void pollJoysticks(std::vector<Event>& events);
    void pollGamepads(std::vector<Event>& events);

    std::array<JoystickSnapshot, MaxJoysticks> previousJoysticks_{};
    std::array<JoystickSnapshot, MaxJoysticks> currentJoysticks_{};
    std::array<GamepadState, MaxGamepads> previousGamepads_{};
    std::array<GamepadState, MaxGamepads> currentGamepads_{};
    std::array<bool, MaxGamepads> previousGamepadPresent_{};
};

std::optional<GamepadState> readStandardGamepadState(uint32_t gamepadId);
std::optional<GamepadInfo> readGamepadInfo(uint32_t gamepadId);

}  // namespace cwin::backend::glfw

#endif
