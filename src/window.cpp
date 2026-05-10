/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#include <cppwindow/window.hpp>

#include <memory>
#include <optional>
#include <span>
#include <string>
#include <utility>

#include "backend/active_backend.hpp"
#include "window_internal.hpp"

namespace cwin {

struct Window::State
{
    explicit State(backend::WindowDesc desc)
        : window(std::move(desc))
    {
    }

    backend::Window window;
};

Window WindowAccess::makeWindow(backend::WindowDesc desc)
{
    return Window(std::make_unique<Window::State>(std::move(desc)));
}

//----------------------------------------------------------------------------
//  Window Implementation
//----------------------------------------------------------------------------
Window::Window(std::unique_ptr<State> state)
    : inputState_(state->window.inputData()),
      state_(std::move(state))
{
}

Window::Window(Window&& other) noexcept
    : inputState_(std::move(other.inputState_)),
      state_(std::move(other.state_))
{
}

Window::~Window() = default;

NativeHandles Window::nativeHandles() const noexcept
{
    return state_->window.nativeHandles();
}

VulkanHandle Window::createVulkanSurface(void* instance) const
{
    return state_->window.createVulkanSurface(instance);
}

void Window::makeContextCurrent() noexcept
{
    state_->window.makeContextCurrent();
}

void Window::swapBuffers() noexcept
{
    state_->window.swapBuffers();
}

bool Window::shouldClose() const noexcept
{
    return state_->window.shouldClose();
}

void Window::requestClose() noexcept
{
    state_->window.requestClose();
}

std::span<const Event> Window::events() const noexcept
{
    return state_->window.events();
}

const InputState& Window::input() const noexcept
{
    return inputState_;
}

void Window::setTitle(const std::string& title) noexcept
{
    state_->window.setTitle(title);
}

std::string Window::title() const
{
    return state_->window.title();
}

void Window::setSize(int width, int height) noexcept
{
    state_->window.setSize(width, height);
}

void Window::setPosition(int x, int y) noexcept
{
    state_->window.setPosition(x, y);
}

void Window::setSizeLimits(const SizeLimits& limits) noexcept
{
    state_->window.setSizeLimits(limits);
}

void Window::clearSizeLimits() noexcept
{
    state_->window.clearSizeLimits();
}

void Window::setAspectRatio(AspectRatio ratio) noexcept
{
    state_->window.setAspectRatio(ratio);
}

void Window::clearAspectRatio() noexcept
{
    state_->window.clearAspectRatio();
}

void Window::setResizable(bool resizable) noexcept
{
    state_->window.setResizable(resizable);
}

void Window::setDecorated(bool decorated) noexcept
{
    state_->window.setDecorated(decorated);
}

void Window::setFloating(bool floating) noexcept
{
    state_->window.setFloating(floating);
}

void Window::setOpacity(float opacity) noexcept
{
    state_->window.setOpacity(opacity);
}

void Window::setVSync(bool enabled) noexcept
{
    state_->window.setVSync(enabled);
}

void Window::setCursorMode(CursorMode mode) noexcept
{
    state_->window.setCursorMode(mode);
}

bool Window::setCursorShape(CursorShape shape) noexcept
{
    return state_->window.setCursorShape(shape);
}

bool Window::setCursorImage(const ImageRgba& image, int hotX, int hotY) noexcept
{
    return state_->window.setCursorImage(image, hotX, hotY);
}

void Window::clearCursor() noexcept
{
    state_->window.clearCursor();
}

void Window::setMousePosition(double x, double y) noexcept
{
    state_->window.setMousePosition(x, y);
}

bool Window::setRawMouseMotion(bool enabled) noexcept
{
    return state_->window.setRawMouseMotion(enabled);
}

void Window::minimize() noexcept
{
    state_->window.minimize();
}

void Window::maximize() noexcept
{
    state_->window.maximize();
}

void Window::restore() noexcept
{
    state_->window.restore();
}

void Window::setWindowMode(
    WindowMode mode,
    uint32_t monitorId,
    std::optional<VideoMode> videoMode) noexcept
{
    state_->window.setWindowMode(mode, monitorId, videoMode);
}

bool Window::setIcon(const ImageRgba& image)
{
    return state_->window.setIcon(std::span<const ImageRgba>{ &image, 1 });
}

bool Window::setIcons(std::span<const ImageRgba> images)
{
    return state_->window.setIcon(images);
}

void Window::clearIcon() noexcept
{
    state_->window.clearIcon();
}

void Window::requestAttention() noexcept
{
    state_->window.requestAttention();
}

void Window::setFocus(bool focus) const noexcept
{
    state_->window.setFocus(focus);
}

void Window::setVisible(bool visible) const noexcept
{
    state_->window.setVisible(visible);
}

WindowPlacement Window::windowedPlacement() const noexcept
{
    return state_->window.windowedPlacement();
}

void Window::setWindowedPlacement(const WindowPlacement& placement) noexcept
{
    state_->window.setWindowedPlacement(placement);
}

std::pair<int, int> Window::size() const noexcept
{
    return state_->window.size();
}

std::pair<int, int> Window::position() const noexcept
{
    return state_->window.position();
}

std::pair<uint32_t, uint32_t> Window::framebufferSize() const noexcept
{
    return state_->window.framebufferSize();
}

std::pair<float, float> Window::contentScale() const noexcept
{
    return state_->window.contentScale();
}

DpiScale Window::dpiScale() const noexcept
{
    auto [x, y] = contentScale();
    return DpiScale{ .x = x, .y = y };
}

float Window::opacity() const noexcept
{
    return state_->window.opacity();
}

CursorMode Window::cursorMode() const noexcept
{
    return state_->window.cursorMode();
}

bool Window::isRawMouseMotionEnabled() const noexcept
{
    return state_->window.isRawMouseMotionEnabled();
}

WindowMode Window::windowMode() const noexcept
{
    return state_->window.windowMode();
}

bool Window::isResizable() const noexcept
{
    return state_->window.isResizable();
}

bool Window::isDecorated() const noexcept
{
    return state_->window.isDecorated();
}

bool Window::isFloating() const noexcept
{
    return state_->window.isFloating();
}

bool Window::isMinimized() const noexcept
{
    return state_->window.isMinimized();
}

bool Window::isMaximized() const noexcept
{
    return state_->window.isMaximized();
}

bool Window::isFocused() const noexcept
{
    return state_->window.isFocused();
}

bool Window::isVisible() const noexcept
{
    return state_->window.isVisible();
}

}  // namespace cwin
