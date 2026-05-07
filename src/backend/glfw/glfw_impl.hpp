/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#ifndef CPPWINDOW_HEADER_GLFW_IMPL_HPP
#define CPPWINDOW_HEADER_GLFW_IMPL_HPP

#include <cppwindow/utils.hpp>

// Prevent GLFW from including OpenGL headers
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <array>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "../../window_registry.hpp"
#include "../native_impl.hpp"

namespace cwin {

//----------------------------------------------------------------------------
//  GLFW Input Mapping
//----------------------------------------------------------------------------
struct GlfwKeyMapTraits
{
    using WrapperType = Key;
    using BackendType = int;

    static constexpr Key WrapperNone = Key::Unknown;
    static constexpr Key WrapperFirst = Key::First;
    static constexpr Key WrapperLast = Key::Last;
    static constexpr int BackendNone = GLFW_KEY_UNKNOWN;
    static constexpr int BackendFirst = GLFW_KEY_SPACE;
    static constexpr int BackendLast = GLFW_KEY_LAST;
};

struct GlfwMouseMapTraits
{
    using WrapperType = MouseButton;
    using BackendType = int;

    static constexpr MouseButton WrapperNone = MouseButton::Unknown;
    static constexpr MouseButton WrapperFirst = MouseButton::First;
    static constexpr MouseButton WrapperLast = MouseButton::Last;
    static constexpr int BackendNone = GLFW_MOUSE_BUTTON_1;
    static constexpr int BackendFirst = GLFW_MOUSE_BUTTON_1;
    static constexpr int BackendLast = GLFW_MOUSE_BUTTON_LAST;
};

using KeyMapLookup = StaticLookup<GlfwKeyMapTraits>;
using MouseMapLookup = StaticLookup<GlfwMouseMapTraits>;

namespace inputmap {

int toGlfwKey(Key k);
Key toKey(int k);
int toGlfwMouseButton(MouseButton b);
MouseButton toMouseButton(int b);

}  // namespace inputmap

//----------------------------------------------------------------------------
//  GLFW Input State
//----------------------------------------------------------------------------
class GLFWInputState
{
public:
    GLFWInputState() = default;
    explicit GLFWInputState(GLFWwindow* window);

    void handleEvent(const Event& event);
    void reset();

    // keyboard
    bool isKeyDown(Key key) const;
    bool isKeyPressed(Key key) const;
    bool isKeyReleased(Key key) const;

    // mouse
    bool isMouseButtonDown(MouseButton button) const;
    bool isMouseButtonPressed(MouseButton button) const;
    bool isMouseButtonReleased(MouseButton button) const;
    std::pair<double, double> mousePosition() const;
    void setMousePosition(double x, double y);
    std::pair<double, double> mouseDelta() const;
    std::pair<double, double> scrollDelta() const;
    bool isMouseInside() const;

    [[nodiscard]] const InputStateData& data() const noexcept;

private:
    bool queryMouseInside() const;

    GLFWwindow* window_ = nullptr;
    InputStateData data_{};
};

//----------------------------------------------------------------------------
//  GLFW Window
//----------------------------------------------------------------------------
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

    void reset()
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

//----------------------------------------------------------------------------
//  GLFW Context
//----------------------------------------------------------------------------
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
    bool setClipboardText(std::string_view text) const;
    std::optional<std::string> clipboardText() const;

private:
    GLFWerrorfun previousErrorCallback_ = nullptr;
};

}  // namespace cwin

#endif
