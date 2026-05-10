/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#ifndef CPPWINDOW_HEADER_GLFW_CONTEXT_HPP
#define CPPWINDOW_HEADER_GLFW_CONTEXT_HPP

#include <cppwindow/core.hpp>
#include <cppwindow/events.hpp>
#include <cppwindow/input.hpp>

#include <GLFW/glfw3.h>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "glfw_gamepad.hpp"

namespace cwin::backend::glfw {

class GLFWWindowContext
{
public:
    GLFWWindowContext();
    ~GLFWWindowContext();

    void pollEvents() noexcept;
    void waitEvents() noexcept;
    void waitEventsTimeout(double timeoutSeconds) noexcept;
    void postEmptyEvent() noexcept;
    std::span<const Event> events() const noexcept;

    ProcLoader procLoader() const noexcept;
    bool isVulkanSupported() const noexcept;
    std::vector<std::string> requiredVulkanExtensions() const;
    std::vector<MonitorInfo> monitors() const;
    std::optional<MonitorInfo> primaryMonitor() const;
    std::vector<VideoMode> videoModes(uint32_t monitorId) const;
    std::pair<float, float> contentScale(uint32_t monitorId) const noexcept;
    std::vector<GamepadInfo> gamepads() const;
    std::optional<GamepadState> gamepadState(uint32_t gamepadId) const;
    bool isRawMouseMotionSupported() const noexcept;
    std::optional<std::string> keyName(Key key, int scancode) const;
    int keyScancode(Key key) const noexcept;
    bool setClipboardText(std::string_view text);
    std::optional<std::string> clipboardText() const;

private:
    void resetEventQueues() noexcept;
    void pollDeviceEvents();

    std::vector<Event> eventQueue_;
    DeviceEventPoller deviceEvents_;
    GLFWerrorfun previousErrorCallback_ = nullptr;
};

}  // namespace cwin::backend::glfw

#endif
