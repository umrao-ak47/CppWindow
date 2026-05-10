/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#ifndef CPPWINDOW_HEADER_GLFW_GAMEPAD_HPP
#define CPPWINDOW_HEADER_GLFW_GAMEPAD_HPP

#include <cppwindow/input.hpp>

#include <cstdint>
#include <optional>

namespace cwin::backend::glfw {

std::optional<GamepadState> readStandardGamepadState(uint32_t gamepadId);
std::optional<GamepadInfo> readGamepadInfo(uint32_t gamepadId);
void pollJoysticks();
void pollGamepads();

}  // namespace cwin::backend::glfw

#endif
