/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#include "glfw_gamepad.hpp"
#include <cppwindow/events.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "glfw_input_map.hpp"
#include "glfw_registry.hpp"

namespace cwin::backend::glfw {

std::optional<GamepadState> readStandardGamepadState(uint32_t gamepadId)
{
    if (gamepadId >= MaxGamepads || !glfwJoystickPresent(static_cast<int>(gamepadId))) {
        return std::nullopt;
    }

    const int joystickId = static_cast<int>(gamepadId);
    if (glfwJoystickIsGamepad(joystickId) != GLFW_TRUE) {
        return std::nullopt;
    }

    GLFWgamepadstate glfwState{};
    if (glfwGetGamepadState(joystickId, &glfwState) != GLFW_TRUE) {
        return std::nullopt;
    }

    GamepadState state{};
    state.id = gamepadId;
    state.standardMapping = true;

    if (const char* name = glfwGetGamepadName(joystickId)) {
        state.name = name;
    } else if (const char* name = glfwGetJoystickName(joystickId)) {
        state.name = name;
    }

    for (size_t i = 0; i < GamepadButtonCount; ++i) {
        state.buttons[i] =
            glfwState.buttons[toGlfwGamepadButton(toGamepadButton(static_cast<int>(i)))] ==
            GLFW_PRESS;
    }

    for (size_t i = 0; i < GamepadAxisCount; ++i) {
        state.axes[i] = glfwState.axes[toGlfwGamepadAxis(toGamepadAxis(static_cast<int>(i)))];
    }

    return state;
}

std::optional<GamepadInfo> readGamepadInfo(uint32_t gamepadId)
{
    if (gamepadId >= MaxGamepads || !glfwJoystickPresent(static_cast<int>(gamepadId))) {
        return std::nullopt;
    }

    const int joystickId = static_cast<int>(gamepadId);
    GamepadInfo info{};
    info.id = gamepadId;
    info.standardMapping = glfwJoystickIsGamepad(joystickId) == GLFW_TRUE;

    const char* name =
        info.standardMapping ? glfwGetGamepadName(joystickId) : glfwGetJoystickName(joystickId);
    if (!name) {
        name = glfwGetJoystickName(joystickId);
    }
    if (name) {
        info.name = name;
    }

    return info;
}

struct JoystickSnapshot
{
    std::string name;
    bool standardMapping = false;
    std::vector<unsigned char> buttons;
    std::vector<float> axes;
};

std::optional<JoystickSnapshot> readJoystickSnapshot(uint32_t joystickId)
{
    if (joystickId >= MaxJoysticks || !glfwJoystickPresent(static_cast<int>(joystickId))) {
        return std::nullopt;
    }

    const int backendId = static_cast<int>(joystickId);
    JoystickSnapshot snapshot{};
    snapshot.standardMapping = glfwJoystickIsGamepad(backendId) == GLFW_TRUE;

    if (const char* name = glfwGetJoystickName(backendId)) {
        snapshot.name = name;
    }

    int buttonCount = 0;
    if (const unsigned char* buttons = glfwGetJoystickButtons(backendId, &buttonCount)) {
        snapshot.buttons.assign(buttons, buttons + buttonCount);
    }

    int axisCount = 0;
    if (const float* axes = glfwGetJoystickAxes(backendId, &axisCount)) {
        snapshot.axes.assign(axes, axes + axisCount);
    }

    return snapshot;
}

void pollJoysticks()
{
    static std::array<std::optional<JoystickSnapshot>, MaxJoysticks> previousStates{};
    constexpr float AxisEpsilon = 0.01f;

    for (uint32_t id = 0; id < MaxJoysticks; ++id) {
        auto current = readJoystickSnapshot(id);
        auto& previous = previousStates[id];

        if (current && !previous) {
            dispatchEventToAllWindows(
                Event::JoystickConnected{
                    .joystickId = id,
                    .name = current->name,
                    .standardMapping = current->standardMapping,
                    .axisCount = static_cast<uint32_t>(current->axes.size()),
                    .buttonCount = static_cast<uint32_t>(current->buttons.size()),
                });
        } else if (!current && previous) {
            dispatchEventToAllWindows(
                Event::JoystickDisconnected{
                    .joystickId = id,
                });
        } else if (current && previous) {
            const size_t buttonCount = std::max(current->buttons.size(), previous->buttons.size());
            for (size_t button = 0; button < buttonCount; ++button) {
                const bool currentDown =
                    button < current->buttons.size() && current->buttons[button] == GLFW_PRESS;
                const bool previousDown =
                    button < previous->buttons.size() && previous->buttons[button] == GLFW_PRESS;
                if (currentDown == previousDown) {
                    continue;
                }

                if (currentDown) {
                    dispatchEventToAllWindows(
                        Event::JoystickButtonPressed{
                            .joystickId = id,
                            .button = static_cast<uint32_t>(button),
                        });
                } else {
                    dispatchEventToAllWindows(
                        Event::JoystickButtonReleased{
                            .joystickId = id,
                            .button = static_cast<uint32_t>(button),
                        });
                }
            }

            const size_t axisCount = std::max(current->axes.size(), previous->axes.size());
            for (size_t axis = 0; axis < axisCount; ++axis) {
                const float currentValue = axis < current->axes.size() ? current->axes[axis] : 0.0f;
                const float previousValue =
                    axis < previous->axes.size() ? previous->axes[axis] : 0.0f;
                if (std::abs(currentValue - previousValue) <= AxisEpsilon) {
                    continue;
                }

                dispatchEventToAllWindows(
                    Event::JoystickMoved{
                        .joystickId = id,
                        .axis = static_cast<uint32_t>(axis),
                        .position = currentValue,
                    });
            }
        }

        previous = std::move(current);
    }
}

void pollGamepads()
{
    static std::array<std::optional<GamepadState>, MaxGamepads> previousStates{};
    constexpr float AxisEpsilon = 0.01f;

    for (uint32_t id = 0; id < MaxGamepads; ++id) {
        auto current = readStandardGamepadState(id);
        auto& previous = previousStates[id];

        if (current && !previous) {
            dispatchEventToAllWindows(
                Event::GamepadConnected{
                    .gamepadId = id,
                    .name = current->name,
                    .standardMapping = current->standardMapping,
                });
        } else if (!current && previous) {
            dispatchEventToAllWindows(
                Event::GamepadDisconnected{
                    .gamepadId = id,
                });
        } else if (current && previous) {
            for (size_t button = 0; button < GamepadButtonCount; ++button) {
                if (current->buttons[button] == previous->buttons[button]) {
                    continue;
                }

                if (current->buttons[button]) {
                    dispatchEventToAllWindows(
                        Event::GamepadButtonPressed{
                            .gamepadId = id,
                            .button = toGamepadButton(static_cast<int>(button)),
                        });
                } else {
                    dispatchEventToAllWindows(
                        Event::GamepadButtonReleased{
                            .gamepadId = id,
                            .button = toGamepadButton(static_cast<int>(button)),
                        });
                }
            }

            for (size_t axis = 0; axis < GamepadAxisCount; ++axis) {
                const float currentValue = current->axes[axis];
                const float previousValue = previous->axes[axis];
                if (std::abs(currentValue - previousValue) <= AxisEpsilon) {
                    continue;
                }

                dispatchEventToAllWindows(
                    Event::GamepadAxisMoved{
                        .gamepadId = id,
                        .axis = toGamepadAxis(static_cast<int>(axis)),
                        .value = currentValue,
                    });
            }
        }

        previous = std::move(current);
    }
}

}  // namespace cwin::backend::glfw
