/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#ifndef CPPWINDOW_HEADER_GLFW_CALLBACKS_HPP
#define CPPWINDOW_HEADER_GLFW_CALLBACKS_HPP

struct GLFWwindow;

namespace cwin::backend {
struct WindowDesc;
}

namespace cwin::backend::glfw {

void setupGlfwWindowHints(const WindowDesc& desc);
void registerGlfwCallbacks(GLFWwindow* handle);

}  // namespace cwin::backend::glfw

#endif
