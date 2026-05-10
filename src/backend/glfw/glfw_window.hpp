/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#ifndef CPPWINDOW_HEADER_GLFW_WINDOW_HPP
#define CPPWINDOW_HEADER_GLFW_WINDOW_HPP

#include <GLFW/glfw3.h>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "../window_desc.hpp"
#include "glfw_input_state.hpp"

namespace cwin::backend::glfw {

struct GLFWwindowDeleter
{
    void operator()(GLFWwindow* window) const noexcept
    {
        if (window) {
            glfwDestroyWindow(window);
        }
    }
};

using UniqueGLFWwindow = std::unique_ptr<GLFWwindow, GLFWwindowDeleter>;

struct GLFWcursorDeleter
{
    void operator()(GLFWcursor* cursor) const noexcept
    {
        if (cursor) {
            glfwDestroyCursor(cursor);
        }
    }
};

using UniqueGLFWcursor = std::unique_ptr<GLFWcursor, GLFWcursorDeleter>;

class WindowStorage
{
public:
    explicit WindowStorage(GLFWwindow* window)
        : inputState(window)
    {
        eventQueue.reserve(64);
    }

    std::vector<Event> eventQueue;
    GLFWInputState inputState;

    void reset() noexcept
    {
        eventQueue.clear();
        inputState.reset();
    }
};

class GLFWNativeWindow
{
public:
    explicit GLFWNativeWindow(WindowDesc desc);
    ~GLFWNativeWindow() = default;

    void handleEvent(Event&& event);
    void handleMonitorChanged(uint32_t monitorId);
    void handleWindowedBoundsChanged();
    void registerCallbacks();

    NativeHandles nativeHandles() const noexcept;
    VulkanHandle createVulkanSurface(void* instance) const;
    void makeContextCurrent() noexcept;
    void swapBuffers() noexcept;

    bool shouldClose() const noexcept;
    void requestClose() noexcept;

    std::span<const Event> events() const noexcept;
    const InputStateData* inputData() const noexcept;

    void setTitle(const std::string& title) noexcept;
    std::string title() const;
    void setSize(int width, int height) noexcept;
    void setPosition(int x, int y) noexcept;
    void setSizeLimits(const SizeLimits& limits) noexcept;
    void clearSizeLimits() noexcept;
    void setAspectRatio(AspectRatio ratio) noexcept;
    void clearAspectRatio() noexcept;
    void setResizable(bool resizable) noexcept;
    void setDecorated(bool decorated) noexcept;
    void setFloating(bool floating) noexcept;
    void setOpacity(float opacity) noexcept;
    void setVSync(bool enabled) noexcept;
    void setCursorMode(CursorMode mode) noexcept;
    bool setCursorShape(CursorShape shape) noexcept;
    bool setCursorImage(const ImageRgba& image, int hotX, int hotY) noexcept;
    void clearCursor() noexcept;
    void setMousePosition(double x, double y) noexcept;
    bool setRawMouseMotion(bool enabled) noexcept;
    void minimize() noexcept;
    void maximize() noexcept;
    void restore() noexcept;
    void
    setWindowMode(WindowMode mode, uint32_t monitorId, std::optional<VideoMode> videoMode) noexcept;
    bool setIcon(std::span<const ImageRgba> images);
    void clearIcon() noexcept;
    void requestAttention() noexcept;
    void setFocus(bool focus) const noexcept;
    void setVisible(bool visible) const noexcept;
    WindowPlacement windowedPlacement() const noexcept;
    void setWindowedPlacement(const WindowPlacement& placement) noexcept;
    std::pair<int, int> size() const noexcept;
    std::pair<int, int> position() const noexcept;
    std::pair<uint32_t, uint32_t> framebufferSize() const noexcept;
    std::pair<float, float> contentScale() const noexcept;
    float opacity() const noexcept;
    CursorMode cursorMode() const noexcept;
    bool isRawMouseMotionEnabled() const noexcept;
    WindowMode windowMode() const noexcept;
    bool isResizable() const noexcept;
    bool isDecorated() const noexcept;
    bool isFloating() const noexcept;
    bool isMinimized() const noexcept;
    bool isMaximized() const noexcept;
    bool isFocused() const noexcept;
    bool isVisible() const noexcept;

private:
    void captureWindowedBounds() noexcept;

    UniqueGLFWwindow handle_{};
    UniqueGLFWcursor cursor_{};
    std::shared_ptr<WindowStorage> storage_{};
    CursorMode cursorMode_ = CursorMode::Normal;
    WindowMode windowMode_ = WindowMode::Windowed;
    bool hasOpenGLContext_ = false;
    bool decorated_ = true;
    bool floating_ = false;
    bool windowedDecorated_ = true;
    bool windowedFloating_ = false;
    uint32_t currentMonitorId_ = 0;
    int windowedX_ = 0;
    int windowedY_ = 0;
    int windowedWidth_ = 1280;
    int windowedHeight_ = 720;
};

}  // namespace cwin::backend::glfw

#endif
