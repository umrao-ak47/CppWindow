/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#ifndef CPPWINDOW_HEADER_GLFW_INPUT_MAP_HPP
#define CPPWINDOW_HEADER_GLFW_INPUT_MAP_HPP

#include <cppwindow/input.hpp>

#include <GLFW/glfw3.h>

namespace cwin::backend::glfw {

int toGlfwKey(Key key);
Key toKey(int key);
int toGlfwMouseButton(MouseButton button);
MouseButton toMouseButton(int button);
int toGlfwGamepadButton(GamepadButton button);
GamepadButton toGamepadButton(int button);
int toGlfwGamepadAxis(GamepadAxis axis);
GamepadAxis toGamepadAxis(int axis);

inline Modifiers toModifiers(int mods) noexcept
{
    return Modifiers{
        .alt = (mods & GLFW_MOD_ALT) != 0,
        .control = (mods & GLFW_MOD_CONTROL) != 0,
        .shift = (mods & GLFW_MOD_SHIFT) != 0,
        .system = (mods & GLFW_MOD_SUPER) != 0,
    };
}

}  // namespace cwin::backend::glfw

#endif
