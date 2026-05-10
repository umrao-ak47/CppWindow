/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#include <cppwindow/cppwindow.hpp>

#include <cmath>

namespace cwin {

//----------------------------------------------------------------------------
//  DPI Implementation
//----------------------------------------------------------------------------
namespace {

[[nodiscard]] double sanitizedScale(float scale) noexcept
{
    return scale > 0.0f && std::isfinite(scale) ? static_cast<double>(scale) : 1.0;
}

}  // namespace

std::pair<double, double> DpiScale::windowToFramebuffer(double xPos, double yPos) const noexcept
{
    return {
        xPos * sanitizedScale(x),
        yPos * sanitizedScale(y),
    };
}

std::pair<double, double> DpiScale::framebufferToWindow(double xPos, double yPos) const noexcept
{
    return {
        xPos / sanitizedScale(x),
        yPos / sanitizedScale(y),
    };
}

std::pair<int, int> DpiScale::windowSizeToFramebuffer(int width, int height) const noexcept
{
    return {
        static_cast<int>(std::lround(width * sanitizedScale(x))),
        static_cast<int>(std::lround(height * sanitizedScale(y))),
    };
}

std::pair<double, double> DpiScale::framebufferSizeToWindow(double width, double height)
    const noexcept
{
    return framebufferToWindow(width, height);
}

}  // namespace cwin
