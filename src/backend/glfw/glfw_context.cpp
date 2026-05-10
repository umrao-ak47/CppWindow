/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#include "glfw_context.hpp"

#include <algorithm>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "glfw_error.hpp"
#include "glfw_gamepad.hpp"
#include "glfw_input_map.hpp"
#include "glfw_monitor.hpp"
#include "glfw_registry.hpp"

namespace cwin::backend::glfw {

GLFWWindowContext::GLFWWindowContext()
{
    previousErrorCallback_ = glfwSetErrorCallback(glfwErrorCallback);
    clearGlfwError();

    if (!glfwInit()) {
        GLFWerrorfun previous = previousErrorCallback_;
        previousErrorCallback_ = nullptr;
        glfwSetErrorCallback(previous);
        throw makeGlfwError(ErrorCode::InitializationFailed, "Failed to initialize GLFW");
    }
}

GLFWWindowContext::~GLFWWindowContext()
{
    glfwTerminate();
    glfwSetErrorCallback(previousErrorCallback_);
}

void GLFWWindowContext::pollEvents() noexcept
{
    // clear old event buffers
    resetWindowStorage();
    // poll new events
    glfwPollEvents();
    pollJoysticks();
    pollGamepads();
}

void GLFWWindowContext::waitEvents() noexcept
{
    resetWindowStorage();
    glfwWaitEvents();
    pollJoysticks();
    pollGamepads();
}

void GLFWWindowContext::waitEventsTimeout(double timeoutSeconds) noexcept
{
    resetWindowStorage();
    glfwWaitEventsTimeout(std::max(0.0, timeoutSeconds));
    pollJoysticks();
    pollGamepads();
}

void GLFWWindowContext::postEmptyEvent() noexcept
{
    glfwPostEmptyEvent();
}

ProcLoader GLFWWindowContext::procLoader() const
{
    return [](const char* name) -> ProcFunction {
        return glfwGetProcAddress(name);
    };
}

bool GLFWWindowContext::isVulkanSupported() const
{
    return glfwVulkanSupported();
}

std::vector<std::string> GLFWWindowContext::requiredVulkanExtensions() const
{
    uint32_t count = 0;
    const char** ext = glfwGetRequiredInstanceExtensions(&count);
    if (!ext) {
        return {};
    }

    return std::vector<std::string>(ext, ext + count);
}

std::vector<MonitorInfo> GLFWWindowContext::monitors() const
{
    std::vector<MonitorInfo> infos;
    auto monitors = getOrderedMonitors();
    infos.reserve(monitors.size());

    for (size_t i = 0; i < monitors.size(); ++i) {
        infos.push_back(toMonitorInfo(monitors[i], static_cast<uint32_t>(i), i == 0));
    }

    return infos;
}

std::optional<MonitorInfo> GLFWWindowContext::primaryMonitor() const
{
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    if (!primary) {
        return std::nullopt;
    }

    return toMonitorInfo(primary, 0, true);
}

std::vector<VideoMode> GLFWWindowContext::videoModes(uint32_t monitorId) const
{
    GLFWmonitor* monitor = getMonitorById(monitorId);
    if (!monitor) {
        return {};
    }

    int count = 0;
    const GLFWvidmode* modes = glfwGetVideoModes(monitor, &count);
    if (!modes || count <= 0) {
        return {};
    }

    std::vector<VideoMode> result;
    result.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        result.push_back(toVideoMode(modes[i]));
    }
    return result;
}

std::pair<float, float> GLFWWindowContext::contentScale(uint32_t monitorId) const
{
    GLFWmonitor* monitor = getMonitorById(monitorId);
    if (!monitor) {
        return { 1.0f, 1.0f };
    }

    float xScale = 1.0f;
    float yScale = 1.0f;
    glfwGetMonitorContentScale(monitor, &xScale, &yScale);
    return { xScale, yScale };
}

std::vector<GamepadInfo> GLFWWindowContext::gamepads() const
{
    std::vector<GamepadInfo> gamepads;
    gamepads.reserve(MaxGamepads);

    for (uint32_t id = 0; id < MaxGamepads; ++id) {
        if (auto info = readGamepadInfo(id)) {
            gamepads.push_back(std::move(*info));
        }
    }

    return gamepads;
}

std::optional<GamepadState> GLFWWindowContext::gamepadState(uint32_t gamepadId) const
{
    return readStandardGamepadState(gamepadId);
}

bool GLFWWindowContext::isRawMouseMotionSupported() const
{
    return glfwRawMouseMotionSupported() == GLFW_TRUE;
}

std::optional<std::string> GLFWWindowContext::keyName(Key key, int scancode) const
{
    const char* name = glfwGetKeyName(toGlfwKey(key), scancode);
    if (!name) {
        return std::nullopt;
    }

    return std::string{ name };
}

int GLFWWindowContext::keyScancode(Key key) const noexcept
{
    const int glfwKey = toGlfwKey(key);
    if (glfwKey == GLFW_KEY_UNKNOWN) {
        return -1;
    }

    return glfwGetKeyScancode(glfwKey);
}

bool GLFWWindowContext::setClipboardText(std::string_view text) const
{
    const std::string ownedText{ text };
    clearGlfwError();
    glfwSetClipboardString(nullptr, ownedText.c_str());
    return takeGlfwError().code == GLFW_NO_ERROR;
}

std::optional<std::string> GLFWWindowContext::clipboardText() const
{
    clearGlfwError();
    const char* text = glfwGetClipboardString(nullptr);
    if (text) {
        (void)takeGlfwError();
        return std::string(text);
    }

    return takeGlfwError().code == GLFW_NO_ERROR ? std::optional<std::string>{ std::string() }
                                                 : std::nullopt;
}

}  // namespace cwin::backend::glfw
