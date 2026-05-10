/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#include <cppwindow/cppwindow.hpp>

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "backend/active_backend.hpp"

namespace cwin {

//----------------------------------------------------------------------------
//  Context Implementation
//----------------------------------------------------------------------------
struct Context::Impl
{
    backend::Context context;
};

Context& Context::get()
{
    // init context
    static Context instance;
    return instance;
}

Context::Context()
    : impl_(std::make_unique<Impl>())
{
}

Context::~Context() = default;

void Context::pollEvents() noexcept
{
    impl_->context.pollEvents();
}

void Context::waitEvents() noexcept
{
    impl_->context.waitEvents();
}

void Context::waitEventsTimeout(double timeoutSeconds) noexcept
{
    impl_->context.waitEventsTimeout(timeoutSeconds);
}

void Context::postEmptyEvent() noexcept
{
    impl_->context.postEmptyEvent();
}

std::span<const Event> Context::events() const noexcept
{
    return impl_->context.events();
}

ProcLoader Context::procLoader() const noexcept
{
    return impl_->context.procLoader();
}

bool Context::isVulkanSupported() const noexcept
{
    return impl_->context.isVulkanSupported();
}

std::vector<std::string> Context::requiredVulkanInstanceExtensions() const
{
    return impl_->context.requiredVulkanExtensions();
}

std::vector<MonitorInfo> Context::monitors() const
{
    return impl_->context.monitors();
}

std::optional<MonitorInfo> Context::primaryMonitor() const
{
    return impl_->context.primaryMonitor();
}

std::vector<VideoMode> Context::videoModes(uint32_t monitorId) const
{
    return impl_->context.videoModes(monitorId);
}

std::pair<float, float> Context::contentScale(uint32_t monitorId) const noexcept
{
    return impl_->context.contentScale(monitorId);
}

DpiScale Context::dpiScale(uint32_t monitorId) const noexcept
{
    auto [x, y] = contentScale(monitorId);
    return DpiScale{ .x = x, .y = y };
}

std::vector<GamepadInfo> Context::gamepads() const
{
    return impl_->context.gamepads();
}

std::optional<GamepadState> Context::gamepadState(uint32_t gamepadId) const
{
    return impl_->context.gamepadState(gamepadId);
}

bool Context::isRawMouseMotionSupported() const noexcept
{
    return impl_->context.isRawMouseMotionSupported();
}

std::optional<std::string> Context::keyName(Key key, int scancode) const
{
    return impl_->context.keyName(key, scancode);
}

int Context::keyScancode(Key key) const noexcept
{
    return impl_->context.keyScancode(key);
}

bool Context::setClipboardText(std::string_view text)
{
    return impl_->context.setClipboardText(text);
}

bool Context::hasClipboardText() const
{
    const auto text = clipboardText();
    return text.has_value() && !text->empty();
}

std::optional<std::string> Context::clipboardText() const
{
    return impl_->context.clipboardText();
}

}  // namespace cwin
