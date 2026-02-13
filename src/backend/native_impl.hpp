/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#pragma once

#include <cppwindow/cppwindow.hpp>

#include <memory>
#include <span>
#include <string>
#include <variant>

namespace cppwindow {

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
    bool resizable;
    bool visible;
    bool decorated;
    bool focused;
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
    virtual std::pair<double, double> getScrollDelta() const = 0;
};

//----------------------------------------------------------------------------
//  Native Window
//----------------------------------------------------------------------------
class NativeWindow
{
public:
    virtual ~NativeWindow() = default;

    virtual VulkanHandle createVulkanSurface(void* instance) const = 0;
    virtual void makeContextCurrent() = 0;
    virtual void swapBuffers() = 0;

    virtual bool shouldClose() const noexcept = 0;
    virtual void requestClose() noexcept = 0;

    virtual std::span<Event> events() const noexcept = 0;
    virtual const NativeInputState* getInput() const noexcept = 0;

    virtual void setTitle(const std::string& title) = 0;
    virtual void setSize(int width, int height) = 0;
    virtual std::pair<int, int> getSize() const noexcept = 0;
    virtual std::pair<uint32_t, uint32_t> getFrameBufferSize() const noexcept = 0;
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

    virtual ProcLoader getProcLoader() const = 0;
    virtual bool isVulkanSupported() const = 0;
    virtual std::vector<std::string> getRequiredVulkanExtensions() const = 0;
};

//----------------------------------------------------------------------------
//  Native Factory
//----------------------------------------------------------------------------
namespace factory {

std::unique_ptr<NativeWindowContext> createNativeContext();
std::unique_ptr<NativeWindow> createNativeWindow(WindowDesc builder);

}  // namespace factory

}  // namespace cppwindow
