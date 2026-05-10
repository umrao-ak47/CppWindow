/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#include "window_internal.hpp"

#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace cwin {

//----------------------------------------------------------------------------
//  Window Builder Implementation
//----------------------------------------------------------------------------
WindowBuilder::WindowBuilder()
    : data_(std::make_unique<Data>()) {};

WindowBuilder::~WindowBuilder() = default;

WindowBuilder& WindowBuilder::title(std::string t)
{
    data_->title = std::move(t);
    return *this;
}

WindowBuilder& WindowBuilder::size(int w, int h)
{
    data_->width = w;
    data_->height = h;
    return *this;
}

WindowBuilder& WindowBuilder::position(int x, int y)
{
    data_->position = std::pair<int, int>{ x, y };
    return *this;
}

WindowBuilder& WindowBuilder::openGL(OpenGLConfig cfg)
{
    data_->mode = OpenGLGraphicsModeTag{
        .config = cfg,
    };
    return *this;
}

WindowBuilder& WindowBuilder::noGraphicsApi()
{
    data_->mode = NoneGraphicsModeTag{};
    return *this;
}

WindowBuilder& WindowBuilder::visible(bool visible)
{
    data_->visible = visible;
    if (!visible) {
        data_->focused = false;
    }
    return *this;
}

WindowBuilder& WindowBuilder::hidden()
{
    return visible(false);
}

WindowBuilder& WindowBuilder::resizable(bool resizable)
{
    data_->resizable = resizable;
    return *this;
}

WindowBuilder& WindowBuilder::focused(bool focused)
{
    data_->focused = focused;
    return *this;
}

WindowBuilder& WindowBuilder::decorated(bool decorated)
{
    data_->decorated = decorated;
    return *this;
}

WindowBuilder& WindowBuilder::borderless()
{
    return decorated(false);
}

WindowBuilder& WindowBuilder::floating(bool floating)
{
    data_->floating = floating;
    return *this;
}

WindowBuilder& WindowBuilder::opacity(float opacity)
{
    data_->opacity = opacity;
    return *this;
}

WindowBuilder& WindowBuilder::sizeLimits(const SizeLimits& limits)
{
    data_->sizeLimits = limits;
    return *this;
}

WindowBuilder& WindowBuilder::aspectRatio(AspectRatio ratio)
{
    data_->aspectRatio = ratio;
    return *this;
}

WindowBuilder& WindowBuilder::cursorMode(CursorMode mode)
{
    data_->cursorMode = mode;
    return *this;
}

WindowBuilder& WindowBuilder::rawMouseMotion(bool enabled)
{
    data_->rawMouseMotion = enabled;
    return *this;
}

WindowBuilder& WindowBuilder::vSync(bool enabled)
{
    data_->vSync = enabled;
    return *this;
}

WindowBuilder&
WindowBuilder::windowMode(WindowMode mode, uint32_t monitorId, std::optional<VideoMode> videoMode)
{
    data_->windowMode = mode;
    data_->monitorId = monitorId;
    data_->videoMode = videoMode;
    return *this;
}

Window WindowBuilder::build()
{
    WindowDesc desc{
        .mode = data_->mode,
        .title = data_->title,
        .width = data_->width,
        .height = data_->height,
        .position = data_->position,
        .resizable = data_->resizable,
        .visible = data_->visible,
        .decorated = data_->decorated,
        .focused = data_->focused,
        .floating = data_->floating,
        .opacity = data_->opacity,
        .sizeLimits = data_->sizeLimits,
        .aspectRatio = data_->aspectRatio,
        .cursorMode = data_->cursorMode,
        .rawMouseMotion = data_->rawMouseMotion,
        .vSync = data_->vSync,
        .windowMode = data_->windowMode,
        .monitorId = data_->monitorId,
        .videoMode = data_->videoMode,
    };
    auto impl = std::make_unique<Window::Impl>(std::move(desc));
    return Window(std::move(impl));
}

}  // namespace cwin
