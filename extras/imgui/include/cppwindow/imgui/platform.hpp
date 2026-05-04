/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

/// @file platform.hpp
/// @brief Optional Dear ImGui platform backend for CppWindow.

#ifndef CPPWINDOW_HEADER_IMGUI_PLATFORM_HPP
#define CPPWINDOW_HEADER_IMGUI_PLATFORM_HPP

#include <chrono>
#include <span>
#include <string>

#include <cppwindow/cppwindow.hpp>

namespace cwin::imgui {

/// Dear ImGui platform backend backed by `cwin::Window` events and state.
///
/// Create a `Context` or app-owned Dear ImGui context before constructing this
/// object. This backend does not render draw data; pair it with a renderer
/// adapter provided by the application, or use `Layer` to coordinate both
/// pieces.
class Platform final
{
public:
    /// Initializes the platform backend for the current ImGui context.
    explicit Platform(Window& window);
    /// Shuts down the platform backend for the current ImGui context.
    ~Platform();

    Platform(const Platform&) = delete;
    Platform& operator=(const Platform&) = delete;
    Platform(Platform&&) = delete;
    Platform& operator=(Platform&&) = delete;

    /// Forwards CppWindow events to Dear ImGui.
    void handleEvents(std::span<const Event> events);
    /// Updates display size, framebuffer scale, delta time, and cursor shape.
    void newFrame();

    /// Returns whether ImGui wants mouse input for the current frame.
    [[nodiscard]] bool wantsMouse() const noexcept;
    /// Returns whether ImGui wants keyboard input for the current frame.
    [[nodiscard]] bool wantsKeyboard() const noexcept;
    /// Returns whether ImGui wants text input for the current frame.
    [[nodiscard]] bool wantsTextInput() const noexcept;

    /// Enables or disables automatic OS cursor shape updates.
    void setMouseCursorUpdatesEnabled(bool enabled) noexcept;
    /// Returns whether automatic OS cursor shape updates are enabled.
    [[nodiscard]] bool mouseCursorUpdatesEnabled() const noexcept;

private:
    static const char* getClipboardText(void* userData);
    static void setClipboardText(void* userData, const char* text);

    void updateMouseCursor();

    Window* window_ = nullptr;
    bool updateMouseCursor_ = true;
    bool hasFrameTime_ = false;
    std::chrono::steady_clock::time_point lastFrameTime_{};
    std::string clipboardText_;
};

}  // namespace cwin::imgui

#endif  // CPPWINDOW_HEADER_IMGUI_PLATFORM_HPP
