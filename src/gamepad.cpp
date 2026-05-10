/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#include <cppwindow/cppwindow.hpp>

#include <cstddef>

namespace cwin {

//----------------------------------------------------------------------------
//  Gamepad State Implementation
//----------------------------------------------------------------------------
namespace {

[[nodiscard]] bool isValidGamepadButton(GamepadButton button) noexcept
{
    const auto idx = static_cast<size_t>(button);
    return idx < GamepadButtonCount;
}

[[nodiscard]] bool isValidGamepadAxis(GamepadAxis axis) noexcept
{
    const auto idx = static_cast<size_t>(axis);
    return idx < GamepadAxisCount;
}

}  // namespace

bool GamepadState::isButtonDown(GamepadButton button) const noexcept
{
    if (!isValidGamepadButton(button)) {
        return false;
    }

    return buttons[static_cast<size_t>(button)];
}

float GamepadState::axis(GamepadAxis axis) const noexcept
{
    if (!isValidGamepadAxis(axis)) {
        return 0.0f;
    }

    return axes[static_cast<size_t>(axis)];
}

}  // namespace cwin
