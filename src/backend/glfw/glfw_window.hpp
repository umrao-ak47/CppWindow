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

    NativeHandles nativeHandles() const;
    VulkanHandle createVulkanSurface(void* instance) const;
    void makeContextCurrent();
    void swapBuffers();

    bool shouldClose() const noexcept;
    void requestClose() noexcept;

    std::span<const Event> events() const noexcept;
    const InputStateData* inputData() const noexcept;

    void setTitle(const std::string& title);
    std::string title() const;
    void setSize(int width, int height);
    void setPosition(int x, int y);
    void setSizeLimits(const SizeLimits& limits);
    void clearSizeLimits();
    void setAspectRatio(AspectRatio ratio);
    void clearAspectRatio();
    void setResizable(bool resizable);
    void setDecorated(bool decorated);
    void setFloating(bool floating);
    void setOpacity(float opacity);
    void setVSync(bool enabled);
    void setCursorMode(CursorMode mode);
    bool setCursorShape(CursorShape shape);
    bool setCursorImage(const ImageRgba& image, int hotX, int hotY);
    void clearCursor();
    void setMousePosition(double x, double y);
    bool setRawMouseMotion(bool enabled);
    void minimize();
    void maximize();
    void restore();
    void setWindowMode(WindowMode mode, uint32_t monitorId, std::optional<VideoMode> videoMode);
    bool setIcon(std::span<const ImageRgba> images);
    void clearIcon();
    void requestAttention();
    void setFocus(bool focus) const noexcept;
    void setVisible(bool visible) const noexcept;
    WindowPlacement windowedPlacement() const noexcept;
    void setWindowedPlacement(const WindowPlacement& placement);
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
    void captureWindowedBounds();

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
