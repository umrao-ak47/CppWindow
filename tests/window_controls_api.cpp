#include <cppwindow/cppwindow.hpp>

#include <cassert>

int main()
{
    cwin::SizeLimits limits{
        .minWidth = 320,
        .minHeight = 180,
        .maxWidth = 1920,
        .maxHeight = 1080,
    };
    cwin::AspectRatio ratio{ 16, 9 };
    cwin::VideoMode mode{
        .width = 1920,
        .height = 1080,
        .redBits = 8,
        .greenBits = 8,
        .blueBits = 8,
        .refreshRate = 60,
    };
    cwin::MonitorInfo monitor{
        .id = 0,
        .name = "Primary",
        .currentVideoMode = mode,
        .primary = true,
    };

    assert(limits.minWidth == 320);
    assert(ratio.numerator == 16);
    assert(monitor.currentVideoMode.refreshRate == 60);
    assert(cwin::CursorMode::Captured != cwin::CursorMode::Normal);
    assert(cwin::WindowMode::BorderlessFullscreen != cwin::WindowMode::Windowed);
    assert(cwin::WindowMode::ExclusiveFullscreen != cwin::WindowMode::Fullscreen);
}
