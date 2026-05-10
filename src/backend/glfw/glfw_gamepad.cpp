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

namespace cwin::backend::glfw {

namespace {

void resetGamepadState(GamepadState& state)
{
    state.id = 0;
    state.name.clear();
    state.standardMapping = false;
    state.buttons.fill(false);
    state.axes.fill(0.0f);
}

bool readStandardGamepadState(uint32_t gamepadId, GamepadState& state)
{
    resetGamepadState(state);
    if (gamepadId >= MaxGamepads || !glfwJoystickPresent(static_cast<int>(gamepadId))) {
        return false;
    }

    const int joystickId = static_cast<int>(gamepadId);
    if (glfwJoystickIsGamepad(joystickId) != GLFW_TRUE) {
        return false;
    }

    GLFWgamepadstate glfwState{};
    if (glfwGetGamepadState(joystickId, &glfwState) != GLFW_TRUE) {
        return false;
    }

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

    return true;
}

}  // namespace

std::optional<GamepadState> readStandardGamepadState(uint32_t gamepadId)
{
    GamepadState state{};
    if (!readStandardGamepadState(gamepadId, state)) {
        return std::nullopt;
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

namespace {

constexpr size_t InitialJoystickButtonCapacity = 32;
constexpr size_t InitialJoystickAxisCapacity = 16;
constexpr size_t InitialDeviceNameCapacity = 64;

}  // namespace

DeviceEventPoller::DeviceEventPoller()
{
    for (auto& snapshot : previousJoysticks_) {
        snapshot.reserveStorage();
    }
    for (auto& snapshot : currentJoysticks_) {
        snapshot.reserveStorage();
    }
    for (auto& gamepad : previousGamepads_) {
        gamepad.name.reserve(InitialDeviceNameCapacity);
    }
    for (auto& gamepad : currentGamepads_) {
        gamepad.name.reserve(InitialDeviceNameCapacity);
    }
}

void DeviceEventPoller::JoystickSnapshot::reset() noexcept
{
    present = false;
    standardMapping = false;
    name.clear();
    buttons.clear();
    axes.clear();
}

void DeviceEventPoller::JoystickSnapshot::reserveStorage()
{
    name.reserve(InitialDeviceNameCapacity);
    buttons.reserve(InitialJoystickButtonCapacity);
    axes.reserve(InitialJoystickAxisCapacity);
}

void DeviceEventPoller::readJoystickSnapshot(uint32_t joystickId, JoystickSnapshot& snapshot)
{
    snapshot.reset();
    if (joystickId >= MaxJoysticks || !glfwJoystickPresent(static_cast<int>(joystickId))) {
        return;
    }

    const int backendId = static_cast<int>(joystickId);
    snapshot.present = true;
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
}

void DeviceEventPoller::poll(std::vector<Event>& events)
{
    pollJoysticks(events);
    pollGamepads(events);
}

void DeviceEventPoller::pollJoysticks(std::vector<Event>& events)
{
    constexpr float AxisEpsilon = 0.01f;

    for (uint32_t id = 0; id < MaxJoysticks; ++id) {
        auto& current = currentJoysticks_[id];
        readJoystickSnapshot(id, current);
        auto& previous = previousJoysticks_[id];

        if (current.present && !previous.present) {
            events.emplace_back(
                Event::JoystickConnected{
                    .joystickId = id,
                    .name = current.name,
                    .standardMapping = current.standardMapping,
                    .axisCount = static_cast<uint32_t>(current.axes.size()),
                    .buttonCount = static_cast<uint32_t>(current.buttons.size()),
                });
        } else if (!current.present && previous.present) {
            events.emplace_back(
                Event::JoystickDisconnected{
                    .joystickId = id,
                });
        } else if (current.present && previous.present) {
            const size_t buttonCount = std::max(current.buttons.size(), previous.buttons.size());
            for (size_t button = 0; button < buttonCount; ++button) {
                const bool currentDown =
                    button < current.buttons.size() && current.buttons[button] == GLFW_PRESS;
                const bool previousDown =
                    button < previous.buttons.size() && previous.buttons[button] == GLFW_PRESS;
                if (currentDown == previousDown) {
                    continue;
                }

                if (currentDown) {
                    events.emplace_back(
                        Event::JoystickButtonPressed{
                            .joystickId = id,
                            .button = static_cast<uint32_t>(button),
                        });
                } else {
                    events.emplace_back(
                        Event::JoystickButtonReleased{
                            .joystickId = id,
                            .button = static_cast<uint32_t>(button),
                        });
                }
            }

            const size_t axisCount = std::max(current.axes.size(), previous.axes.size());
            for (size_t axis = 0; axis < axisCount; ++axis) {
                const float currentValue = axis < current.axes.size() ? current.axes[axis] : 0.0f;
                const float previousValue =
                    axis < previous.axes.size() ? previous.axes[axis] : 0.0f;
                if (std::abs(currentValue - previousValue) <= AxisEpsilon) {
                    continue;
                }

                events.emplace_back(
                    Event::JoystickMoved{
                        .joystickId = id,
                        .axis = static_cast<uint32_t>(axis),
                        .position = currentValue,
                    });
            }
        }

        std::swap(previous, current);
    }
}

void DeviceEventPoller::pollGamepads(std::vector<Event>& events)
{
    constexpr float AxisEpsilon = 0.01f;

    for (uint32_t id = 0; id < MaxGamepads; ++id) {
        auto& current = currentGamepads_[id];
        auto& previous = previousGamepads_[id];
        const bool currentPresent = readStandardGamepadState(id, current);
        const bool wasPresent = previousGamepadPresent_[id];

        if (currentPresent && !wasPresent) {
            events.emplace_back(
                Event::GamepadConnected{
                    .gamepadId = id,
                    .name = current.name,
                    .standardMapping = current.standardMapping,
                });
        } else if (!currentPresent && wasPresent) {
            events.emplace_back(
                Event::GamepadDisconnected{
                    .gamepadId = id,
                });
        } else if (currentPresent && wasPresent) {
            for (size_t button = 0; button < GamepadButtonCount; ++button) {
                if (current.buttons[button] == previous.buttons[button]) {
                    continue;
                }

                if (current.buttons[button]) {
                    events.emplace_back(
                        Event::GamepadButtonPressed{
                            .gamepadId = id,
                            .button = toGamepadButton(static_cast<int>(button)),
                        });
                } else {
                    events.emplace_back(
                        Event::GamepadButtonReleased{
                            .gamepadId = id,
                            .button = toGamepadButton(static_cast<int>(button)),
                        });
                }
            }

            for (size_t axis = 0; axis < GamepadAxisCount; ++axis) {
                const float currentValue = current.axes[axis];
                const float previousValue = previous.axes[axis];
                if (std::abs(currentValue - previousValue) <= AxisEpsilon) {
                    continue;
                }

                events.emplace_back(
                    Event::GamepadAxisMoved{
                        .gamepadId = id,
                        .axis = toGamepadAxis(static_cast<int>(axis)),
                        .value = currentValue,
                    });
            }
        }

        previousGamepadPresent_[id] = currentPresent;
        std::swap(previous, current);
    }
}

}  // namespace cwin::backend::glfw
