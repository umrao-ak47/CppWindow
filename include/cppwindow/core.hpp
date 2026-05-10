/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

/// @file core.hpp
/// @brief Core CppWindow types, errors, native handles, and window metadata.

#ifndef CPPWINDOW_HEADER_CORE_HPP
#define CPPWINDOW_HEADER_CORE_HPP

#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace cwin {

/// Public error category for failures reported by CppWindow.
enum class ErrorCode : uint8_t
{
    /// Backend failure that does not fit a narrower category.
    BackendFailure,
    /// Failed to initialize the backend/windowing system.
    InitializationFailed,
    /// Failed to create a native window.
    WindowCreationFailed,
    /// Failed to create a Vulkan surface for a window.
    VulkanSurfaceCreationFailed
};

/// Exception thrown for unrecoverable CppWindow API failures.
class Error : public std::runtime_error
{
public:
    /// Creates an error with a category and human-readable message.
    Error(ErrorCode code, std::string message);

    /// Returns the error category.
    [[nodiscard]] ErrorCode code() const noexcept;

private:
    ErrorCode code_;
};

/// Generic function pointer returned by a graphics procedure loader.
using ProcFunction = void (*)();

/// Function pointer type used to resolve graphics/backend procedures by name.
using ProcLoader = ProcFunction (*)(const char*);

/// Integer wrapper for Vulkan handles without exposing Vulkan headers.
using VulkanHandle = uint64_t;

/// Platform-specific native handles for integration boundaries.
struct NativeHandles
{
    /// Native window system that produced the handles.
    enum class System
    {
        /// Win32 window handle.
        Win32,
        /// Cocoa NSWindow handle.
        Cocoa,
        /// X11 window/display handles.
        X11,
        /// Wayland surface/display handles.
        Wayland,
        /// Unknown or unsupported native window system.
        Unknown
    };

    /// Native window system.
    System system = System::Unknown;
    /// Native window pointer or handle. Type depends on `system`.
    void* window = nullptr;
    /// Native display pointer when the platform exposes one.
    void* display = nullptr;
};

/// Graphics API requested at window creation.
enum class GraphicsMode : uint8_t
{
    /// No graphics API. Use this for Vulkan or custom surface setup.
    None,
    /// OpenGL context.
    OpenGL
};

/// OpenGL context creation request.
struct OpenGLConfig
{
    /// Requested OpenGL major version.
    int major = 4;
    /// Requested OpenGL minor version.
    int minor = 5;
    /// Whether to request an OpenGL core profile.
    bool coreProfile = true;
};

/// Cursor visibility/capture mode.
enum class CursorMode : uint8_t
{
    /// Normal visible OS cursor.
    Normal,
    /// Hidden cursor while over the window.
    Hidden,
    /// Captured cursor for relative camera/mouse-look input.
    Captured
};

/// Standard platform cursor shapes.
enum class CursorShape : uint8_t
{
    /// Default arrow cursor.
    Arrow,
    /// Text insertion cursor.
    IBeam,
    /// Crosshair cursor.
    Crosshair,
    /// Pointing hand cursor.
    Hand,
    /// Horizontal resize cursor.
    ResizeHorizontal,
    /// Vertical resize cursor.
    ResizeVertical,
    /// Northwest/southeast diagonal resize cursor.
    ResizeDiagonalNWSE,
    /// Northeast/southwest diagonal resize cursor.
    ResizeDiagonalNESW,
    /// Omnidirectional resize cursor.
    ResizeAll,
    /// Unavailable/not-allowed cursor.
    NotAllowed
};

/// 8-bit RGBA image data for window icons and custom cursors.
struct ImageRgba
{
    /// Image width in pixels.
    uint32_t width = 0;
    /// Image height in pixels.
    uint32_t height = 0;
    /// Pixel bytes in RGBA order. Must contain at least `width * height * 4` bytes.
    std::span<const uint8_t> pixels;
};

/// High-level window presentation mode.
enum class WindowMode : uint8_t
{
    /// Normal restored window.
    Windowed,
    /// Decorated maximized window with title bar.
    Fullscreen,
    /// Undecorated window covering the monitor work area.
    BorderlessFullscreen,
    /// True monitor fullscreen that may change display mode/content scale.
    ExclusiveFullscreen
};

/// Monitor video mode.
struct VideoMode
{
    /// Width in screen/video-mode pixels.
    int width = 0;
    /// Height in screen/video-mode pixels.
    int height = 0;
    /// Red color bits.
    int redBits = 0;
    /// Green color bits.
    int greenBits = 0;
    /// Blue color bits.
    int blueBits = 0;
    /// Refresh rate in Hz.
    int refreshRate = 0;
};

/// Monitor metadata returned by `WindowContext`.
struct MonitorInfo
{
    /// Stable id for the current monitor list ordering.
    uint32_t id = 0;
    /// Human-readable monitor name when available.
    std::string name;
    /// Monitor x position in virtual desktop coordinates.
    int x = 0;
    /// Monitor y position in virtual desktop coordinates.
    int y = 0;
    /// Physical width in millimeters when available.
    int physicalWidthMM = 0;
    /// Physical height in millimeters when available.
    int physicalHeightMM = 0;
    /// Horizontal content scale.
    float contentScaleX = 1.0f;
    /// Vertical content scale.
    float contentScaleY = 1.0f;
    /// Current video mode.
    VideoMode currentVideoMode;
    /// Whether this is the primary monitor.
    bool primary = false;
};

/// Optional window size limits.
struct SizeLimits
{
    /// Minimum width, or empty for no minimum.
    std::optional<int> minWidth;
    /// Minimum height, or empty for no minimum.
    std::optional<int> minHeight;
    /// Maximum width, or empty for no maximum.
    std::optional<int> maxWidth;
    /// Maximum height, or empty for no maximum.
    std::optional<int> maxHeight;
};

/// Window content aspect ratio.
struct AspectRatio
{
    /// Ratio numerator, usually width.
    int numerator = 1;
    /// Ratio denominator, usually height.
    int denominator = 1;
};

/// Saved or requested placement for a restored window.
struct WindowPlacement
{
    /// Window x position in virtual desktop coordinates.
    int x = 0;
    /// Window y position in virtual desktop coordinates.
    int y = 0;
    /// Window content width in screen coordinates.
    int width = 0;
    /// Window content height in screen coordinates.
    int height = 0;
    /// Whether the restored window should be maximized.
    bool maximized = false;
};

/// Window-to-framebuffer content scale helper.
struct DpiScale
{
    /// Horizontal scale from window coordinates to framebuffer pixels.
    float x = 1.0f;
    /// Vertical scale from window coordinates to framebuffer pixels.
    float y = 1.0f;

    /// Converts a position from window coordinates to framebuffer pixels.
    [[nodiscard]] std::pair<double, double> windowToFramebuffer(double x, double y) const noexcept;
    /// Converts a position from framebuffer pixels to window coordinates.
    [[nodiscard]] std::pair<double, double> framebufferToWindow(double x, double y) const noexcept;
    /// Converts a size from window coordinates to framebuffer pixels.
    [[nodiscard]] std::pair<int, int> windowSizeToFramebuffer(int width, int height) const noexcept;
    /// Converts a size from framebuffer pixels to window coordinates.
    [[nodiscard]] std::pair<double, double> framebufferSizeToWindow(double width, double height)
        const noexcept;
};

}  // namespace cwin

#endif
