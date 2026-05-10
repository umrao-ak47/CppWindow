/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#ifndef CPPWINDOW_HEADER_GLFW_MONITOR_HPP
#define CPPWINDOW_HEADER_GLFW_MONITOR_HPP

#include <cppwindow/core.hpp>

#include <GLFW/glfw3.h>
#include <cstdint>
#include <vector>

namespace cwin::backend::glfw {

std::vector<GLFWmonitor*> getOrderedMonitors();
GLFWmonitor* getMonitorById(uint32_t monitorId) noexcept;
VideoMode toVideoMode(const GLFWvidmode& mode) noexcept;
MonitorInfo toMonitorInfo(GLFWmonitor* monitor, uint32_t id, bool primary);
uint32_t getWindowMonitorId(GLFWwindow* window);

}  // namespace cwin::backend::glfw

#endif
