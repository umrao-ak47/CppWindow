/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#include "window_internal.hpp"

#include <memory>
#include <optional>
#include <span>
#include <string>
#include <utility>

namespace cwin {

//----------------------------------------------------------------------------
//  Window Implementation
//----------------------------------------------------------------------------
Window::Window(std::unique_ptr<Impl> impl)
    : inputState_(impl->window.inputData()),
      impl_(std::move(impl))
{
}

Window::Window(Window&& other) noexcept
    : inputState_(std::move(other.inputState_)),
      impl_(std::move(other.impl_))
{
}

Window::~Window() = default;

NativeHandles Window::nativeHandles() const
{
    return impl_->window.nativeHandles();
}

VulkanHandle Window::createVulkanSurface(void* instance) const
{
    return impl_->window.createVulkanSurface(instance);
}

void Window::makeContextCurrent()
{
    impl_->window.makeContextCurrent();
}

void Window::swapBuffers()
{
    impl_->window.swapBuffers();
}

bool Window::shouldClose() const noexcept
{
    return impl_->window.shouldClose();
}

void Window::requestClose() noexcept
{
    impl_->window.requestClose();
}

std::span<const Event> Window::events() const noexcept
{
    return impl_->window.events();
}

const InputState& Window::input() const noexcept
{
    return inputState_;
}

void Window::setTitle(const std::string& title)
{
    impl_->window.setTitle(title);
}

std::string Window::title() const
{
    return impl_->window.title();
}

void Window::setSize(int width, int height)
{
    impl_->window.setSize(width, height);
}

void Window::setPosition(int x, int y)
{
    impl_->window.setPosition(x, y);
}

void Window::setSizeLimits(const SizeLimits& limits)
{
    impl_->window.setSizeLimits(limits);
}

void Window::clearSizeLimits()
{
    impl_->window.clearSizeLimits();
}

void Window::setAspectRatio(AspectRatio ratio)
{
    impl_->window.setAspectRatio(ratio);
}

void Window::clearAspectRatio()
{
    impl_->window.clearAspectRatio();
}

void Window::setResizable(bool resizable)
{
    impl_->window.setResizable(resizable);
}

void Window::setDecorated(bool decorated)
{
    impl_->window.setDecorated(decorated);
}

void Window::setFloating(bool floating)
{
    impl_->window.setFloating(floating);
}

void Window::setOpacity(float opacity)
{
    impl_->window.setOpacity(opacity);
}

void Window::setVSync(bool enabled)
{
    impl_->window.setVSync(enabled);
}

void Window::setCursorMode(CursorMode mode)
{
    impl_->window.setCursorMode(mode);
}

bool Window::setCursorShape(CursorShape shape)
{
    return impl_->window.setCursorShape(shape);
}

bool Window::setCursorImage(const ImageRgba& image, int hotX, int hotY)
{
    return impl_->window.setCursorImage(image, hotX, hotY);
}

void Window::clearCursor()
{
    impl_->window.clearCursor();
}

void Window::setMousePosition(double x, double y)
{
    impl_->window.setMousePosition(x, y);
}

bool Window::setRawMouseMotion(bool enabled)
{
    return impl_->window.setRawMouseMotion(enabled);
}

void Window::minimize()
{
    impl_->window.minimize();
}

void Window::maximize()
{
    impl_->window.maximize();
}

void Window::restore()
{
    impl_->window.restore();
}

void Window::setWindowMode(WindowMode mode, uint32_t monitorId, std::optional<VideoMode> videoMode)
{
    impl_->window.setWindowMode(mode, monitorId, videoMode);
}

bool Window::setIcon(const ImageRgba& image)
{
    return impl_->window.setIcon(std::span<const ImageRgba>{ &image, 1 });
}

bool Window::setIcons(std::span<const ImageRgba> images)
{
    return impl_->window.setIcon(images);
}

void Window::clearIcon()
{
    impl_->window.clearIcon();
}

void Window::requestAttention()
{
    impl_->window.requestAttention();
}

void Window::setFocus(bool focus) const noexcept
{
    impl_->window.setFocus(focus);
}

void Window::setVisible(bool visible) const noexcept
{
    impl_->window.setVisible(visible);
}

WindowPlacement Window::windowedPlacement() const noexcept
{
    return impl_->window.windowedPlacement();
}

void Window::setWindowedPlacement(const WindowPlacement& placement)
{
    impl_->window.setWindowedPlacement(placement);
}

std::pair<int, int> Window::size() const noexcept
{
    return impl_->window.size();
}

std::pair<int, int> Window::position() const noexcept
{
    return impl_->window.position();
}

std::pair<uint32_t, uint32_t> Window::framebufferSize() const noexcept
{
    return impl_->window.framebufferSize();
}

std::pair<float, float> Window::contentScale() const noexcept
{
    return impl_->window.contentScale();
}

DpiScale Window::dpiScale() const noexcept
{
    auto [x, y] = contentScale();
    return DpiScale{ .x = x, .y = y };
}

float Window::opacity() const noexcept
{
    return impl_->window.opacity();
}

CursorMode Window::cursorMode() const noexcept
{
    return impl_->window.cursorMode();
}

bool Window::isRawMouseMotionEnabled() const noexcept
{
    return impl_->window.isRawMouseMotionEnabled();
}

WindowMode Window::windowMode() const noexcept
{
    return impl_->window.windowMode();
}

bool Window::isResizable() const noexcept
{
    return impl_->window.isResizable();
}

bool Window::isDecorated() const noexcept
{
    return impl_->window.isDecorated();
}

bool Window::isFloating() const noexcept
{
    return impl_->window.isFloating();
}

bool Window::isMinimized() const noexcept
{
    return impl_->window.isMinimized();
}

bool Window::isMaximized() const noexcept
{
    return impl_->window.isMaximized();
}

bool Window::isFocused() const noexcept
{
    return impl_->window.isFocused();
}

bool Window::isVisible() const noexcept
{
    return impl_->window.isVisible();
}

}  // namespace cwin
