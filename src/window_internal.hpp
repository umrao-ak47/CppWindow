/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#ifndef CPPWINDOW_HEADER_WINDOW_INTERNAL_HPP
#define CPPWINDOW_HEADER_WINDOW_INTERNAL_HPP

#include "backend/active_backend.hpp"

#include <optional>
#include <string>
#include <utility>

namespace cwin {

struct Window::Impl
{
    explicit Impl(WindowDesc desc)
        : window(std::move(desc))
    {
    }

    backend::Window window;
};

struct WindowBuilder::Data
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

}  // namespace cwin

#endif
