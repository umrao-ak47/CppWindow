/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#ifndef CPPWINDOW_HEADER_WINDOW_DESC_HPP
#define CPPWINDOW_HEADER_WINDOW_DESC_HPP

#include <cppwindow/core.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
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

}  // namespace cwin

#endif
