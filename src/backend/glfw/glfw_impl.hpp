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
    std::pair<double, double> getMousePosition() const;
    void setMousePosition(double x, double y);
    std::pair<double, double> getMouseDelta() const;
    std::pair<double, double> getScrollDelta() const;
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

class GLFWNativeWindow : public NativeWindow
{
public:
    explicit GLFWNativeWindow(WindowDesc desc);
    ~GLFWNativeWindow() = default;

    void handleEvent(Event&& event);
    void handleMonitorChanged(uint32_t monitorId);
    void registerCallbacks();

    NativeHandles getNativeHandles() const override;
    VulkanHandle createVulkanSurface(void* instance) const override;
    void makeContextCurrent() override;
    void swapBuffers() override;

    bool shouldClose() const noexcept override;
    void requestClose() noexcept override;

    std::span<const Event> events() const noexcept override;
    const InputStateData* getInputData() const noexcept override;

    void setTitle(const std::string& title) override;
    void setSize(int width, int height) override;
    void setPosition(int x, int y) override;
    void setSizeLimits(const SizeLimits& limits) override;
    void clearSizeLimits() override;
    void setAspectRatio(AspectRatio ratio) override;
    void clearAspectRatio() override;
    void setResizable(bool resizable) override;
    void setDecorated(bool decorated) override;
    void setFloating(bool floating) override;
    void setOpacity(float opacity) override;
    void setVSync(bool enabled) override;
    void setCursorMode(CursorMode mode) override;
    bool setCursorShape(CursorShape shape) override;
    bool setCursorImage(const ImageRgba& image, int hotX, int hotY) override;
    void clearCursor() override;
    void setMousePosition(double x, double y) override;
    bool setRawMouseMotion(bool enabled) override;
    void minimize() override;
    void maximize() override;
    void restore() override;
    void setWindowMode(WindowMode mode, uint32_t monitorId, std::optional<VideoMode> videoMode)
        override;
    bool setIcon(std::span<const ImageRgba> images) override;
    void clearIcon() override;
    void requestAttention() override;
    void setFocus(bool focus) const noexcept override;
    void setVisible(bool visible) const noexcept override;
    std::pair<int, int> getSize() const noexcept override;
    std::pair<int, int> getPosition() const noexcept override;
    std::pair<uint32_t, uint32_t> getFramebufferSize() const noexcept override;
    std::pair<float, float> getContentScale() const noexcept override;
    float getOpacity() const noexcept override;
    CursorMode getCursorMode() const noexcept override;
    bool isRawMouseMotionEnabled() const noexcept override;
    WindowMode getWindowMode() const noexcept override;
    bool isResizable() const noexcept override;
    bool isDecorated() const noexcept override;
    bool isFloating() const noexcept override;
    bool isMinimized() const noexcept override;
    bool isMaximized() const noexcept override;
    bool isFocused() const noexcept override;
    bool isVisible() const noexcept override;

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
class GLFWWindowContext : public NativeWindowContext
{
public:
    GLFWWindowContext();
    ~GLFWWindowContext();

    void pollEvents() noexcept override;
    void waitEvents() noexcept override;
    void waitEventsTimeout(double timeoutSeconds) noexcept override;
    void postEmptyEvent() noexcept override;

    ProcLoader getProcLoader() const override;
    bool isVulkanSupported() const override;
    std::vector<std::string> getRequiredVulkanExtensions() const override;
    std::vector<MonitorInfo> getMonitors() const override;
    std::optional<MonitorInfo> getPrimaryMonitor() const override;
    std::vector<VideoMode> getVideoModes(uint32_t monitorId) const override;
    std::pair<float, float> getContentScale(uint32_t monitorId) const override;
    std::vector<GamepadInfo> getGamepads() const override;
    std::optional<GamepadState> getGamepadState(uint32_t gamepadId) const override;
    bool isRawMouseMotionSupported() const override;
    bool setClipboardText(const std::string& text) const override;
    std::string getClipboardText() const override;
    std::optional<std::string> tryGetClipboardText() const override;

private:
    GLFWerrorfun previousErrorCallback_ = nullptr;
};

}  // namespace cwin

#endif
