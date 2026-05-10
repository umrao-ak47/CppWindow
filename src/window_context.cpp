/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#include <cppwindow/cppwindow.hpp>

#include "backend/active_backend.hpp"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace cwin {

//----------------------------------------------------------------------------
//  Window Context Implemenation
//----------------------------------------------------------------------------
struct WindowContext::Impl
{
    backend::WindowContext context;
};

WindowContext& WindowContext::get()
{
    // init context
    static WindowContext instance;
    return instance;
}

WindowContext::WindowContext()
    : impl_(std::make_unique<Impl>())
{}

WindowContext::~WindowContext() = default;

void WindowContext::pollEvents() const noexcept
{
    impl_->context.pollEvents();
}

void WindowContext::waitEvents() const noexcept
{
    impl_->context.waitEvents();
}

void WindowContext::waitEventsTimeout(double timeoutSeconds) const noexcept
{
    impl_->context.waitEventsTimeout(timeoutSeconds);
}

void WindowContext::postEmptyEvent() const noexcept
{
    impl_->context.postEmptyEvent();
}

ProcLoader WindowContext::procLoader() const
{
    return impl_->context.procLoader();
}

bool WindowContext::isVulkanSupported() const
{
    return impl_->context.isVulkanSupported();
}

std::vector<std::string> WindowContext::requiredVulkanInstanceExtensions() const
{
    return impl_->context.requiredVulkanExtensions();
}

std::vector<MonitorInfo> WindowContext::monitors() const
{
    return impl_->context.monitors();
}

std::optional<MonitorInfo> WindowContext::primaryMonitor() const
{
    return impl_->context.primaryMonitor();
}

std::vector<VideoMode> WindowContext::videoModes(uint32_t monitorId) const
{
    return impl_->context.videoModes(monitorId);
}

std::pair<float, float> WindowContext::contentScale(uint32_t monitorId) const
{
    return impl_->context.contentScale(monitorId);
}

DpiScale WindowContext::dpiScale(uint32_t monitorId) const
{
    auto [x, y] = contentScale(monitorId);
    return DpiScale{ .x = x, .y = y };
}

std::vector<GamepadInfo> WindowContext::gamepads() const
{
    return impl_->context.gamepads();
}

std::optional<GamepadState> WindowContext::gamepadState(uint32_t gamepadId) const
{
    return impl_->context.gamepadState(gamepadId);
}

bool WindowContext::isRawMouseMotionSupported() const
{
    return impl_->context.isRawMouseMotionSupported();
}

std::optional<std::string> WindowContext::keyName(Key key, int scancode) const
{
    return impl_->context.keyName(key, scancode);
}

int WindowContext::keyScancode(Key key) const noexcept
{
    return impl_->context.keyScancode(key);
}

bool WindowContext::setClipboardText(std::string_view text) const
{
    return impl_->context.setClipboardText(text);
}

bool WindowContext::hasClipboardText() const
{
    const auto text = clipboardText();
    return text.has_value() && !text->empty();
}

std::optional<std::string> WindowContext::clipboardText() const
{
    return impl_->context.clipboardText();
}

}  // namespace cwin
