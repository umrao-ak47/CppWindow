/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#ifndef CPPWINDOW_HEADER_GLFW_INTERNAL_HPP
#define CPPWINDOW_HEADER_GLFW_INTERNAL_HPP

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "glfw_window.hpp"

namespace cwin::glfw_backend {

struct GlfwError
{
    int code = GLFW_NO_ERROR;
    std::string description;
};

template <typename... Ts>
struct Overloaded : Ts...
{
    using Ts::operator()...;
};

void glfwErrorCallback(int code, const char* description);
GlfwError takeGlfwError();
void clearGlfwError();
Error makeGlfwError(ErrorCode code, const std::string& message);
Error makeGlfwError(ErrorCode code, const std::string& message, const std::string& detail);

int toGlfwKey(Key key);
Key toKey(int key);
int toGlfwMouseButton(MouseButton button);
MouseButton toMouseButton(int button);

void registerWindowStorage(const std::shared_ptr<WindowStorage>& storage);
void resetWindowStorage() noexcept;
void dispatchEventToAllWindows(const Event& event);
std::vector<GLFWmonitor*> getOrderedMonitors();
GLFWmonitor* getMonitorById(uint32_t monitorId);
VideoMode toVideoMode(const GLFWvidmode& mode);
MonitorInfo toMonitorInfo(GLFWmonitor* monitor, uint32_t id, bool primary);
int toGlfwCursorMode(CursorMode mode);
int toGlfwCursorShape(CursorShape shape);
std::optional<GLFWimage> toGlfwImage(const ImageRgba& image) noexcept;
Modifiers toModifiers(int mods);
uint32_t getWindowMonitorId(GLFWwindow* window);
std::optional<GamepadState> readStandardGamepadState(uint32_t gamepadId);
std::optional<GamepadInfo> readGamepadInfo(uint32_t gamepadId);
void pollJoysticks();
void pollGamepads();

void setupGlfwWindowHints(const WindowDesc& desc);
void registerGlfwCallbacks(GLFWwindow* handle);

}  // namespace cwin::glfw_backend

#endif
