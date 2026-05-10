/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#ifndef CPPWINDOW_HEADER_GLFW_CURSOR_HPP
#define CPPWINDOW_HEADER_GLFW_CURSOR_HPP

#include <cppwindow/core.hpp>

#include <GLFW/glfw3.h>
#include <optional>

namespace cwin::backend::glfw {

int toGlfwCursorMode(CursorMode mode);
int toGlfwCursorShape(CursorShape shape);
std::optional<GLFWimage> toGlfwImage(const ImageRgba& image) noexcept;

}  // namespace cwin::backend::glfw

#endif
