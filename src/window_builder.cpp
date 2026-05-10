/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#include <cppwindow/window.hpp>

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "window_internal.hpp"

namespace cwin {

struct WindowBuilder::State
{
    GraphicsModeTag mode = NoneGraphicsModeTag();
    std::string title = "CppWindow";
    uint32_t width = 1280;
    uint32_t height = 720;
    std::optional<std::pair<int, int>> position;
    bool resizable = false;
    bool visible = true;
    bool decorated = true;
    bool focused = true;
    bool floating = false;
    std::optional<float> opacity;
    std::optional<SizeLimits> sizeLimits;
    std::optional<AspectRatio> aspectRatio;
    std::optional<CursorMode> cursorMode;
    std::optional<bool> rawMouseMotion;
    std::optional<bool> vSync;
    WindowMode windowMode = WindowMode::Windowed;
    uint32_t monitorId = 0;
    std::optional<VideoMode> videoMode;
};

//----------------------------------------------------------------------------
//  Window Builder Implementation
//----------------------------------------------------------------------------
WindowBuilder::WindowBuilder()
    : state_(std::make_unique<State>()) {};

WindowBuilder::~WindowBuilder() = default;

WindowBuilder& WindowBuilder::title(std::string t)
{
    state_->title = std::move(t);
    return *this;
}

WindowBuilder& WindowBuilder::size(int w, int h)
{
    state_->width = w;
    state_->height = h;
    return *this;
}

WindowBuilder& WindowBuilder::position(int x, int y)
{
    state_->position = std::pair<int, int>{ x, y };
    return *this;
}

WindowBuilder& WindowBuilder::openGL(OpenGLConfig cfg)
{
    state_->mode = OpenGLGraphicsModeTag{
        .config = cfg,
    };
    return *this;
}

WindowBuilder& WindowBuilder::noGraphicsApi()
{
    state_->mode = NoneGraphicsModeTag{};
    return *this;
}

WindowBuilder& WindowBuilder::visible(bool visible)
{
    state_->visible = visible;
    if (!visible) {
        state_->focused = false;
    }
    return *this;
}

WindowBuilder& WindowBuilder::hidden()
{
    return visible(false);
}

WindowBuilder& WindowBuilder::resizable(bool resizable)
{
    state_->resizable = resizable;
    return *this;
}

WindowBuilder& WindowBuilder::focused(bool focused)
{
    state_->focused = focused;
    return *this;
}

WindowBuilder& WindowBuilder::decorated(bool decorated)
{
    state_->decorated = decorated;
    return *this;
}

WindowBuilder& WindowBuilder::borderless()
{
    return decorated(false);
}

WindowBuilder& WindowBuilder::floating(bool floating)
{
    state_->floating = floating;
    return *this;
}

WindowBuilder& WindowBuilder::opacity(float opacity)
{
    state_->opacity = opacity;
    return *this;
}

WindowBuilder& WindowBuilder::sizeLimits(const SizeLimits& limits)
{
    state_->sizeLimits = limits;
    return *this;
}

WindowBuilder& WindowBuilder::aspectRatio(AspectRatio ratio)
{
    state_->aspectRatio = ratio;
    return *this;
}

WindowBuilder& WindowBuilder::cursorMode(CursorMode mode)
{
    state_->cursorMode = mode;
    return *this;
}

WindowBuilder& WindowBuilder::rawMouseMotion(bool enabled)
{
    state_->rawMouseMotion = enabled;
    return *this;
}

WindowBuilder& WindowBuilder::vSync(bool enabled)
{
    state_->vSync = enabled;
    return *this;
}

WindowBuilder&
WindowBuilder::windowMode(WindowMode mode, uint32_t monitorId, std::optional<VideoMode> videoMode)
{
    state_->windowMode = mode;
    state_->monitorId = monitorId;
    state_->videoMode = videoMode;
    return *this;
}

Window WindowBuilder::build()
{
    WindowDesc desc{
        .mode = state_->mode,
        .title = state_->title,
        .width = state_->width,
        .height = state_->height,
        .position = state_->position,
        .resizable = state_->resizable,
        .visible = state_->visible,
        .decorated = state_->decorated,
        .focused = state_->focused,
        .floating = state_->floating,
        .opacity = state_->opacity,
        .sizeLimits = state_->sizeLimits,
        .aspectRatio = state_->aspectRatio,
        .cursorMode = state_->cursorMode,
        .rawMouseMotion = state_->rawMouseMotion,
        .vSync = state_->vSync,
        .windowMode = state_->windowMode,
        .monitorId = state_->monitorId,
        .videoMode = state_->videoMode,
    };
    return WindowAccess::makeWindow(std::move(desc));
}

}  // namespace cwin
