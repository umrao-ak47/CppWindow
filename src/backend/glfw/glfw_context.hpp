/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#ifndef CPPWINDOW_HEADER_GLFW_CONTEXT_HPP
#define CPPWINDOW_HEADER_GLFW_CONTEXT_HPP

#include <cppwindow/core.hpp>
#include <cppwindow/input.hpp>

#include <GLFW/glfw3.h>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

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

    ProcLoader procLoader() const;
    bool isVulkanSupported() const;
    std::vector<std::string> requiredVulkanExtensions() const;
    std::vector<MonitorInfo> monitors() const;
    std::optional<MonitorInfo> primaryMonitor() const;
    std::vector<VideoMode> videoModes(uint32_t monitorId) const;
    std::pair<float, float> contentScale(uint32_t monitorId) const;
    std::vector<GamepadInfo> gamepads() const;
    std::optional<GamepadState> gamepadState(uint32_t gamepadId) const;
    bool isRawMouseMotionSupported() const;
    std::optional<std::string> keyName(Key key, int scancode) const;
    int keyScancode(Key key) const noexcept;
    bool setClipboardText(std::string_view text) const;
    std::optional<std::string> clipboardText() const;

private:
    GLFWerrorfun previousErrorCallback_ = nullptr;
};

}  // namespace cwin::backend::glfw

#endif
