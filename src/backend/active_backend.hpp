/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#ifndef CPPWINDOW_HEADER_ACTIVE_BACKEND_HPP
#define CPPWINDOW_HEADER_ACTIVE_BACKEND_HPP

#include "glfw/glfw_context.hpp"
#include "glfw/glfw_window.hpp"

namespace cwin::backend {

using Window = glfw::GLFWNativeWindow;
using Context = glfw::GLFWWindowContext;

}  // namespace cwin::backend

#endif  // CPPWINDOW_HEADER_ACTIVE_BACKEND_HPP
