/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#include <cstddef>
#include <limits>
#include <optional>

#include "glfw_internal.hpp"

namespace cwin::glfw_backend {

int toGlfwCursorMode(CursorMode mode)
{
    switch (mode) {
        case CursorMode::Normal:
            return GLFW_CURSOR_NORMAL;
        case CursorMode::Hidden:
            return GLFW_CURSOR_HIDDEN;
        case CursorMode::Captured:
            return GLFW_CURSOR_DISABLED;
    }

    return GLFW_CURSOR_NORMAL;
}

int toGlfwCursorShape(CursorShape shape)
{
    switch (shape) {
        case CursorShape::Arrow:
            return GLFW_ARROW_CURSOR;
        case CursorShape::IBeam:
            return GLFW_IBEAM_CURSOR;
        case CursorShape::Crosshair:
            return GLFW_CROSSHAIR_CURSOR;
        case CursorShape::Hand:
            return GLFW_POINTING_HAND_CURSOR;
        case CursorShape::ResizeHorizontal:
            return GLFW_RESIZE_EW_CURSOR;
        case CursorShape::ResizeVertical:
            return GLFW_RESIZE_NS_CURSOR;
        case CursorShape::ResizeDiagonalNWSE:
            return GLFW_RESIZE_NWSE_CURSOR;
        case CursorShape::ResizeDiagonalNESW:
            return GLFW_RESIZE_NESW_CURSOR;
        case CursorShape::ResizeAll:
            return GLFW_RESIZE_ALL_CURSOR;
        case CursorShape::NotAllowed:
            return GLFW_NOT_ALLOWED_CURSOR;
    }

    return GLFW_ARROW_CURSOR;
}

bool isValidImage(const ImageRgba& image) noexcept
{
    if (image.width == 0 || image.height == 0) {
        return false;
    }

    const size_t maxSize = std::numeric_limits<size_t>::max();
    if (static_cast<size_t>(image.width) > maxSize / static_cast<size_t>(image.height)) {
        return false;
    }

    const size_t pixelCount = static_cast<size_t>(image.width) * static_cast<size_t>(image.height);
    if (pixelCount > maxSize / 4) {
        return false;
    }

    return image.pixels.size() >= pixelCount * 4;
}

std::optional<GLFWimage> toGlfwImage(const ImageRgba& image) noexcept
{
    if (!isValidImage(image)) {
        return std::nullopt;
    }

    if (image.width > static_cast<uint32_t>(std::numeric_limits<int>::max()) ||
        image.height > static_cast<uint32_t>(std::numeric_limits<int>::max())) {
        return std::nullopt;
    }

    return GLFWimage{
        .width = static_cast<int>(image.width),
        .height = static_cast<int>(image.height),
        .pixels = const_cast<unsigned char*>(image.pixels.data()),
    };
}

}  // namespace cwin::glfw_backend
