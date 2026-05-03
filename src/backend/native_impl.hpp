/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#ifndef CPPWINDOW_HEADER_NATIVE_IMPL_HPP
#define CPPWINDOW_HEADER_NATIVE_IMPL_HPP

#include <cppwindow/cppwindow.hpp>

#include <memory>
#include <span>
#include <string>
#include <variant>

namespace cwin {

struct NoneGraphicsModeTag
{};

struct OpenGLGraphicsModeTag
{
    OpenGLConfig config;
};

using GraphicsModeTag = std::variant<NoneGraphicsModeTag, OpenGLGraphicsModeTag>;

struct WindowDesc
{
    GraphicsModeTag mode;
    std::string title;
    uint32_t width;
    uint32_t height;
    std::optional<std::pair<int, int>> position;
    bool resizable;
    bool visible;
    bool decorated;
    bool focused;
    bool floating;
    std::optional<float> opacity;
    std::optional<SizeLimits> sizeLimits;
    std::optional<AspectRatio> aspectRatio;
    std::optional<CursorMode> cursorMode;
    std::optional<bool> rawMouseMotion;
    std::optional<bool> vSync;
    WindowMode windowMode;
    uint32_t monitorId;
    std::optional<VideoMode> videoMode;
};

//----------------------------------------------------------------------------
//  Native InputState
//----------------------------------------------------------------------------
class NativeInputState
{
public:
    NativeInputState() = default;
    virtual ~NativeInputState() = default;

    virtual void handleEvent(const Event& event) = 0;
    virtual void reset() = 0;

    // keyboard
    virtual bool isKeyDown(Key key) const = 0;
    virtual bool isKeyPressed(Key key) const = 0;
    virtual bool isKeyReleased(Key key) const = 0;

    // mouse
    virtual bool isMouseButtonDown(MouseButton button) const = 0;
    virtual bool isMouseButtonPressed(MouseButton button) const = 0;
    virtual bool isMouseButtonReleased(MouseButton button) const = 0;
    virtual std::pair<double, double> getMousePosition() const = 0;
    virtual void setMousePosition(double x, double y) = 0;
    virtual std::pair<double, double> getMouseDelta() const = 0;
    virtual std::pair<double, double> getScrollDelta() const = 0;
    virtual bool isMouseInside() const = 0;
};

//----------------------------------------------------------------------------
//  Native Window
//----------------------------------------------------------------------------
class NativeWindow
{
public:
    virtual ~NativeWindow() = default;

    virtual NativeHandles getNativeHandles() const = 0;
    virtual VulkanHandle createVulkanSurface(void* instance) const = 0;
    virtual void makeContextCurrent() = 0;
    virtual void swapBuffers() = 0;

    virtual bool shouldClose() const noexcept = 0;
    virtual void requestClose() noexcept = 0;

    virtual std::span<const Event> events() const noexcept = 0;
    virtual const NativeInputState* getInput() const noexcept = 0;

    virtual void setTitle(const std::string& title) = 0;
    virtual void setSize(int width, int height) = 0;
    virtual void setPosition(int x, int y) = 0;
    virtual void setSizeLimits(const SizeLimits& limits) = 0;
    virtual void clearSizeLimits() = 0;
    virtual void setAspectRatio(AspectRatio ratio) = 0;
    virtual void clearAspectRatio() = 0;
    virtual void setResizable(bool resizable) = 0;
    virtual void setDecorated(bool decorated) = 0;
    virtual void setFloating(bool floating) = 0;
    virtual void setOpacity(float opacity) = 0;
    virtual void setVSync(bool enabled) = 0;
    virtual void setCursorMode(CursorMode mode) = 0;
    virtual bool setCursorShape(CursorShape shape) = 0;
    virtual bool setCursorImage(const ImageRgba& image, int hotX, int hotY) = 0;
    virtual void clearCursor() = 0;
    virtual void setMousePosition(double x, double y) = 0;
    virtual bool setRawMouseMotion(bool enabled) = 0;
    virtual void minimize() = 0;
    virtual void maximize() = 0;
    virtual void restore() = 0;
    virtual void
    setWindowMode(WindowMode mode, uint32_t monitorId, std::optional<VideoMode> videoMode) = 0;
    virtual bool setIcon(std::span<const ImageRgba> images) = 0;
    virtual void clearIcon() = 0;
    virtual void requestAttention() = 0;
    virtual void setFocus(bool focus) const noexcept = 0;
    virtual void setVisible(bool visible) const noexcept = 0;
    virtual std::pair<int, int> getSize() const noexcept = 0;
    virtual std::pair<int, int> getPosition() const noexcept = 0;
    virtual std::pair<uint32_t, uint32_t> getFramebufferSize() const noexcept = 0;
    virtual std::pair<float, float> getContentScale() const noexcept = 0;
    virtual float getOpacity() const noexcept = 0;
    virtual CursorMode getCursorMode() const noexcept = 0;
    virtual bool isRawMouseMotionEnabled() const noexcept = 0;
    virtual WindowMode getWindowMode() const noexcept = 0;
    virtual bool isResizable() const noexcept = 0;
    virtual bool isDecorated() const noexcept = 0;
    virtual bool isFloating() const noexcept = 0;
    virtual bool isMinimized() const noexcept = 0;
    virtual bool isMaximized() const noexcept = 0;
    virtual bool isFocused() const noexcept = 0;
    virtual bool isVisible() const noexcept = 0;
};

//----------------------------------------------------------------------------
//  Native Context
//----------------------------------------------------------------------------
class NativeWindowContext
{
public:
    virtual ~NativeWindowContext() = default;

    virtual void pollEvents() noexcept = 0;
    virtual void waitEvents() noexcept = 0;
    virtual void waitEventsTimeout(double timeoutSeconds) noexcept = 0;
    virtual void postEmptyEvent() noexcept = 0;

    virtual ProcLoader getProcLoader() const = 0;
    virtual bool isVulkanSupported() const = 0;
    virtual std::vector<std::string> getRequiredVulkanExtensions() const = 0;
    virtual std::vector<MonitorInfo> getMonitors() const = 0;
    virtual std::optional<MonitorInfo> getPrimaryMonitor() const = 0;
    virtual std::vector<VideoMode> getVideoModes(uint32_t monitorId) const = 0;
    virtual std::pair<float, float> getContentScale(uint32_t monitorId) const = 0;
    virtual std::vector<GamepadInfo> getGamepads() const = 0;
    virtual std::optional<GamepadState> getGamepadState(uint32_t gamepadId) const = 0;
    virtual bool isRawMouseMotionSupported() const = 0;
    virtual bool setClipboardText(const std::string& text) const = 0;
    virtual std::string getClipboardText() const = 0;
    virtual std::optional<std::string> tryGetClipboardText() const = 0;
};

//----------------------------------------------------------------------------
//  Native Factory
//----------------------------------------------------------------------------
namespace factory {

std::unique_ptr<NativeWindowContext> createNativeContext();
std::unique_ptr<NativeWindow> createNativeWindow(WindowDesc builder);

}  // namespace factory

}  // namespace cwin

#endif
