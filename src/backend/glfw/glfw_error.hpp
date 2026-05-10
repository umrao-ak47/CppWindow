/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#ifndef CPPWINDOW_HEADER_GLFW_ERROR_HPP
#define CPPWINDOW_HEADER_GLFW_ERROR_HPP

#include <cppwindow/core.hpp>

#include <GLFW/glfw3.h>
#include <string>

namespace cwin::backend::glfw {

struct GlfwError
{
    int code = GLFW_NO_ERROR;
    std::string description;
};

void glfwErrorCallback(int code, const char* description);
GlfwError takeGlfwError();
void clearGlfwError();
Error makeGlfwError(ErrorCode code, const std::string& message);
Error makeGlfwError(ErrorCode code, const std::string& message, const std::string& detail);

}  // namespace cwin::backend::glfw

#endif
