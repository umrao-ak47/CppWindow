/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#include "glfw_internal.hpp"

namespace cwin::glfw_backend {

Modifiers toModifiers(int mods)
{
    return Modifiers{
        .alt = (mods & GLFW_MOD_ALT) != 0,
        .control = (mods & GLFW_MOD_CONTROL) != 0,
        .shift = (mods & GLFW_MOD_SHIFT) != 0,
        .system = (mods & GLFW_MOD_SUPER) != 0,
    };
}

}  // namespace cwin::glfw_backend
