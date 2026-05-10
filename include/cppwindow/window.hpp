/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

/// @file window.hpp
/// @brief Window, window builder, and process-wide window context APIs.

#ifndef CPPWINDOW_HEADER_WINDOW_HPP
#define CPPWINDOW_HEADER_WINDOW_HPP

#include <cppwindow/core.hpp>
#include <cppwindow/events.hpp>
#include <cppwindow/input.hpp>

#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cwin {

//----------------------------------------------------------------------------
//  Window
//----------------------------------------------------------------------------
/// Fluent builder for `Window`.
class WindowBuilder;

/// Native window wrapper with graphics, event, and input controls.
class Window final
{
    friend struct WindowAccess;

public:
    /// Destroys the native window.
    ~Window();
    /// Moves window ownership.
    Window(Window&&) noexcept;
    /// Windows are unique owners and cannot be copied.
    Window(const Window&) = delete;

    /// Windows are unique owners and cannot be copy-assigned.
    Window& operator=(const Window&) = delete;
    /// Move-assignment is disabled because input state references native storage.
    Window& operator=(Window&&) = delete;

    /// Returns platform-native handles for advanced integration.
    NativeHandles nativeHandles() const;
    /// Creates a Vulkan surface for the given native VkInstance pointer.
    VulkanHandle createVulkanSurface(void* instance) const;
    /// Makes this window's OpenGL context current on the calling thread.
    void makeContextCurrent();
    /// Swaps the front and back buffers.
    void swapBuffers();

    /// Returns whether the window should close.
    bool shouldClose() const noexcept;
    /// Requests that the window close.
    void requestClose() noexcept;

    /// Returns read-only events collected by the last context poll.
    std::span<const Event> events() const noexcept;
    /// Returns the input query interface for this window.
    const InputState& input() const noexcept;

    /// Sets the window title.
    void setTitle(const std::string& title);
    /// Returns the current UTF-8 window title.
    std::string title() const;
    /// Sets the content size in screen coordinates.
    void setSize(int width, int height);
    /// Sets the window position in virtual desktop coordinates.
    void setPosition(int x, int y);
    /// Applies optional min/max size limits.
    void setSizeLimits(const SizeLimits& limits);
    /// Removes size limits.
    void clearSizeLimits();
    /// Sets the content aspect ratio.
    void setAspectRatio(AspectRatio ratio);
    /// Removes the content aspect ratio constraint.
    void clearAspectRatio();
    /// Enables or disables user resizing.
    void setResizable(bool resizable);
    /// Enables or disables platform decorations such as title bar and border.
    void setDecorated(bool decorated);
    /// Enables or disables always-on-top behavior.
    void setFloating(bool floating);
    /// Sets window opacity in the platform-supported range.
    void setOpacity(float opacity);
    /// Enables or disables swap interval synchronization.
    void setVSync(bool enabled);
    /// Sets cursor visibility/capture mode.
    void setCursorMode(CursorMode mode);
    /// Sets a standard platform cursor shape. Returns false when unsupported.
    [[nodiscard]] bool setCursorShape(CursorShape shape);
    /// Sets a custom RGBA cursor image with hotspot coordinates. Returns false on invalid input.
    [[nodiscard]] bool setCursorImage(const ImageRgba& image, int hotX = 0, int hotY = 0);
    /// Restores the platform default cursor image.
    void clearCursor();
    /// Sets the cursor position in window coordinates.
    void setMousePosition(double x, double y);
    /// Enables or disables raw mouse motion when supported by the backend.
    [[nodiscard]] bool setRawMouseMotion(bool enabled);
    /// Minimizes the window.
    void minimize();
    /// Maximizes the window.
    void maximize();
    /// Restores the window from minimized, maximized, or fullscreen mode.
    void restore();
    /// Changes the presentation mode, optionally targeting a monitor/video mode.
    void setWindowMode(
        WindowMode mode,
        uint32_t monitorId = 0,
        std::optional<VideoMode> videoMode = std::nullopt);
    /// Sets one window icon image. Returns false when unsupported or invalid.
    [[nodiscard]] bool setIcon(const ImageRgba& image);
    /// Sets multiple window icon images. Returns false when unsupported or invalid.
    [[nodiscard]] bool setIcons(std::span<const ImageRgba> images);
    /// Restores the platform default window icon.
    void clearIcon();
    /// Requests user attention for the window.
    void requestAttention();
    /// Requests or clears input focus where the platform permits it.
    void setFocus(bool focus) const noexcept;
    /// Shows or hides the window.
    void setVisible(bool visible) const noexcept;
    /// Returns the saved/restored windowed placement.
    WindowPlacement windowedPlacement() const noexcept;
    /// Applies a saved/restored windowed placement.
    void setWindowedPlacement(const WindowPlacement& placement);
    /// Returns the content size in screen coordinates.
    std::pair<int, int> size() const noexcept;
    /// Returns the window position in virtual desktop coordinates.
    std::pair<int, int> position() const noexcept;
    /// Returns framebuffer pixel size.
    std::pair<uint32_t, uint32_t> framebufferSize() const noexcept;
    /// Returns content scale for the window.
    std::pair<float, float> contentScale() const noexcept;
    /// Returns DPI/content scale conversion helper for this window.
    DpiScale dpiScale() const noexcept;
    /// Returns current window opacity.
    float opacity() const noexcept;
    /// Returns current cursor mode.
    CursorMode cursorMode() const noexcept;
    /// Returns whether raw mouse motion is enabled for this window.
    bool isRawMouseMotionEnabled() const noexcept;
    /// Returns current presentation mode.
    WindowMode windowMode() const noexcept;
    /// Returns whether user resizing is enabled.
    bool isResizable() const noexcept;
    /// Returns whether platform decorations are enabled.
    bool isDecorated() const noexcept;
    /// Returns whether always-on-top behavior is enabled.
    bool isFloating() const noexcept;
    /// Returns whether the window is minimized/iconified.
    bool isMinimized() const noexcept;
    /// Returns whether the window is maximized.
    bool isMaximized() const noexcept;
    /// Returns whether the window has input focus.
    bool isFocused() const noexcept;
    /// Returns whether the window is visible.
    bool isVisible() const noexcept;

private:
    struct State;

    explicit Window(std::unique_ptr<State> state);

    InputState inputState_;
    std::unique_ptr<State> state_;
};

//----------------------------------------------------------------------------
//  Window Builder
//----------------------------------------------------------------------------
/// Fluent builder used to create a configured `Window`.
class WindowBuilder
{
public:
    /// Creates a builder with default title, size, visibility, and no graphics API.
    WindowBuilder();
    /// Destroys builder state.
    ~WindowBuilder();

    /// Sets the initial window title.
    WindowBuilder& title(std::string t);
    /// Sets the initial content size.
    WindowBuilder& size(int w, int h);
    /// Sets the initial window position in virtual desktop coordinates.
    WindowBuilder& position(int x, int y);
    /// Requests an OpenGL context with the given configuration.
    WindowBuilder& openGL(OpenGLConfig cfg = {});
    /// Requests no graphics API for this window.
    WindowBuilder& noGraphicsApi();
    /// Sets whether the window is initially visible.
    WindowBuilder& visible(bool visible = true);
    /// Creates the window hidden and unfocused.
    WindowBuilder& hidden();
    /// Sets whether the user can resize the window.
    WindowBuilder& resizable(bool resizable = true);
    /// Sets whether the window initially has input focus.
    WindowBuilder& focused(bool focused = true);
    /// Sets whether the window uses platform decorations.
    WindowBuilder& decorated(bool decorated = true);
    /// Creates the window without platform decorations.
    WindowBuilder& borderless();
    /// Sets whether the window is initially always-on-top.
    WindowBuilder& floating(bool floating = true);
    /// Sets initial platform opacity.
    WindowBuilder& opacity(float opacity);
    /// Sets initial min/max size limits.
    WindowBuilder& sizeLimits(const SizeLimits& limits);
    /// Sets initial aspect ratio.
    WindowBuilder& aspectRatio(AspectRatio ratio);
    /// Sets initial cursor visibility/capture mode.
    WindowBuilder& cursorMode(CursorMode mode);
    /// Sets initial raw mouse motion mode when supported by the backend.
    WindowBuilder& rawMouseMotion(bool enabled = true);
    /// Sets initial OpenGL swap interval behavior.
    WindowBuilder& vSync(bool enabled = true);
    /// Sets the initial presentation mode.
    WindowBuilder& windowMode(
        WindowMode mode,
        uint32_t monitorId = 0,
        std::optional<VideoMode> videoMode = std::nullopt);
    /// Creates the window.
    Window build();

private:
    struct State;
    std::unique_ptr<State> state_{};
};

//----------------------------------------------------------------------------
//  Window Context
//----------------------------------------------------------------------------
/// Process-wide windowing context and platform service entry point.
class WindowContext final
{
public:
    /// Returns the process-wide context singleton.
    static WindowContext& get();

    /// Destroys the windowing context after owned windows are gone.
    ~WindowContext();
    /// The context is a singleton and cannot be copied.
    WindowContext(const WindowContext&) = delete;
    /// The context is a singleton and cannot be assigned.
    WindowContext& operator=(const WindowContext&) = delete;

    /// Polls platform events and updates window/input state.
    void pollEvents() const noexcept;
    /// Waits until at least one platform event is available, then updates state.
    void waitEvents() const noexcept;
    /// Waits up to `timeoutSeconds` for events, then updates state.
    void waitEventsTimeout(double timeoutSeconds) const noexcept;
    /// Wakes a thread blocked in `waitEvents` or `waitEventsTimeout`.
    void postEmptyEvent() const noexcept;

    /// Returns a procedure loader for OpenGL or backend integration.
    ProcLoader procLoader() const;
    /// Returns whether Vulkan presentation support is available.
    bool isVulkanSupported() const;
    /// Returns Vulkan instance extensions required by the active window backend.
    std::vector<std::string> requiredVulkanInstanceExtensions() const;
    /// Returns metadata for all connected monitors.
    std::vector<MonitorInfo> monitors() const;
    /// Returns the primary monitor, if one is available.
    std::optional<MonitorInfo> primaryMonitor() const;
    /// Returns video modes for a monitor id, defaulting to the primary monitor.
    std::vector<VideoMode> videoModes(uint32_t monitorId = 0) const;
    /// Returns monitor content scale, defaulting to the primary monitor.
    std::pair<float, float> contentScale(uint32_t monitorId = 0) const;
    /// Returns DPI/content scale conversion helper for a monitor.
    DpiScale dpiScale(uint32_t monitorId = 0) const;
    /// Returns connected standard gamepads.
    std::vector<GamepadInfo> gamepads() const;
    /// Returns the current state for a standard gamepad.
    std::optional<GamepadState> gamepadState(uint32_t gamepadId = 0) const;
    /// Returns whether raw mouse motion is supported by the backend/platform.
    bool isRawMouseMotionSupported() const;
    /// Returns a localized key label for a logical key or platform scancode.
    [[nodiscard]] std::optional<std::string> keyName(Key key, int scancode = -1) const;
    /// Returns the platform scancode for a logical key, or -1 when unavailable.
    [[nodiscard]] int keyScancode(Key key) const noexcept;
    /// Sets the platform clipboard text. Returns false when the backend reports failure.
    [[nodiscard]] bool setClipboardText(std::string_view text) const;
    /// Returns whether clipboard text is currently available and non-empty.
    [[nodiscard]] bool hasClipboardText() const;
    /// Returns platform clipboard text, or null when the backend reports failure.
    [[nodiscard]] std::optional<std::string> clipboardText() const;

private:
    struct Impl;

    WindowContext();

    std::unique_ptr<Impl> impl_{};
};

}  // namespace cwin

#endif
