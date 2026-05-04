/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

/// @file cppwindow.hpp
/// @brief Public API for CppWindow.
///
/// CppWindow is a C++20 platform layer for windows, input, events, monitor
/// queries, graphics context/surface interop, and lightweight app utilities.

#ifndef CPPWINDOW_HEADER_CPPWINDOW_HPP
#define CPPWINDOW_HEADER_CPPWINDOW_HPP

#include <array>
#include <chrono>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
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

/// Keyboard key code.
enum class Key : uint32_t
{
    Unknown = 0,
    First = 1,
    Space = First,
    Apostrophe,
    Comma,
    Minus,
    Period,
    Slash,
    Num0,
    Num1,
    Num2,
    Num3,
    Num4,
    Num5,
    Num6,
    Num7,
    Num8,
    Num9,
    Semicolon,
    Equal,
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    LBracket,
    Backslash,
    RBracket,
    Grave,
    World1,
    World2,
    Escape,
    Enter,
    Tab,
    Backspace,
    Insert,
    Delete,
    Right,
    Left,
    Down,
    Up,
    PageUp,
    PageDown,
    Home,
    End,
    CapsLock,
    ScrollLock,
    NumLock,
    PrintScreen,
    Pause,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    F13,
    F14,
    F15,
    F16,
    F17,
    F18,
    F19,
    F20,
    F21,
    F22,
    F23,
    F24,
    F25,
    Numpad0,
    Numpad1,
    Numpad2,
    Numpad3,
    Numpad4,
    Numpad5,
    Numpad6,
    Numpad7,
    Numpad8,
    Numpad9,
    NumpadDecimal,
    NumpadDivide,
    NumpadMultiply,
    NumpadSubtract,
    NumpadAdd,
    NumpadEnter,
    NumpadEqual,
    LShift,
    LControl,
    LAlt,
    LSuper,
    RShift,
    RControl,
    RAlt,
    RSuper,
    Menu,
    Last = Menu,
};

/// Backend-style input action names.
enum class Action : int
{
    /// Release action.
    Release = 0,
    /// Press action.
    Press,
    /// Repeat action.
    Repeat
};

/// Keyboard or mouse modifier state.
struct Modifiers
{
    /// Whether Alt/Option was held.
    bool alt = false;
    /// Whether Control was held.
    bool control = false;
    /// Whether Shift was held.
    bool shift = false;
    /// Whether the system/super/command modifier was held.
    bool system = false;

    /// Returns whether any modifier is held.
    [[nodiscard]] bool any() const noexcept
    {
        return alt || control || shift || system;
    }

    /// Compares two modifier sets.
    [[nodiscard]] bool operator==(const Modifiers&) const noexcept = default;
};

/// Mouse button code.
enum class MouseButton : uint8_t
{
    /// Unknown mouse button.
    Unknown = 0,
    /// First valid mouse button value.
    First = 1,
    Left = First,   //!< The left mouse button
    Right,          //!< The right mouse button
    Middle,         //!< The middle (wheel) mouse button
    Button4,        //!< The first extra mouse button
    Button5,        //!< The second extra mouse button
    Button6,        //!< The third extra mouse button
    Button7,        //!< The fourth extra mouse button
    Button8,        //!< The fifth extra mouse button
    Last = Button8  // Count for MouseButton
};

/// Number of key slots, including `Key::Unknown`.
inline constexpr std::size_t KeyCount{ static_cast<std::size_t>(Key::Last) + 1 };
/// Number of mouse button slots, including `MouseButton::Unknown`.
inline constexpr std::size_t MouseButtonCount{ static_cast<std::size_t>(MouseButton::Last) + 1 };

/// Standard gamepad button code.
enum class GamepadButton : uint8_t
{
    /// South face button on most controllers.
    A,
    /// East face button on most controllers.
    B,
    /// West face button on most controllers.
    X,
    /// North face button on most controllers.
    Y,
    /// Left shoulder button.
    LeftBumper,
    /// Right shoulder button.
    RightBumper,
    /// Back/select button.
    Back,
    /// Start/menu button.
    Start,
    /// Guide/system button.
    Guide,
    /// Left thumbstick press.
    LeftThumb,
    /// Right thumbstick press.
    RightThumb,
    /// D-pad up.
    DPadUp,
    /// D-pad right.
    DPadRight,
    /// D-pad down.
    DPadDown,
    /// D-pad left.
    DPadLeft,
    /// Last valid standard gamepad button.
    Last = DPadLeft
};

/// Standard gamepad axis code.
enum class GamepadAxis : uint8_t
{
    /// Left stick horizontal axis.
    LeftX,
    /// Left stick vertical axis.
    LeftY,
    /// Right stick horizontal axis.
    RightX,
    /// Right stick vertical axis.
    RightY,
    /// Left trigger axis.
    LeftTrigger,
    /// Right trigger axis.
    RightTrigger,
    /// Last valid standard gamepad axis.
    Last = RightTrigger
};

/// Number of standard gamepad button slots.
inline constexpr std::size_t GamepadButtonCount{
    static_cast<std::size_t>(GamepadButton::Last) + 1
};
/// Number of standard gamepad axis slots.
inline constexpr std::size_t GamepadAxisCount{ static_cast<std::size_t>(GamepadAxis::Last) + 1 };
/// Maximum gamepad slots scanned by the backend.
inline constexpr std::size_t MaxGamepads{ 16 };
/// Maximum joystick slots scanned by the backend.
inline constexpr std::size_t MaxJoysticks{ 16 };

/// Connected gamepad metadata.
struct GamepadInfo
{
    /// Gamepad slot id.
    uint32_t id = 0;
    /// Human-readable device name when available.
    std::string name;
    /// Whether the device has a standard gamepad mapping.
    bool standardMapping = false;
};

/// Snapshot of a standard-mapped gamepad.
struct GamepadState
{
    /// Gamepad slot id.
    uint32_t id = 0;
    /// Human-readable device name when available.
    std::string name;
    /// Whether this state uses a standard mapping.
    bool standardMapping = false;
    /// Button states indexed by `GamepadButton`.
    std::array<bool, GamepadButtonCount> buttons{};
    /// Axis values indexed by `GamepadAxis`.
    std::array<float, GamepadAxisCount> axes{};

    /// Returns whether a standard gamepad button is down.
    [[nodiscard]] bool isButtonDown(GamepadButton button) const noexcept;
    /// Returns the current value for a standard gamepad axis.
    [[nodiscard]] float getAxis(GamepadAxis axis) const noexcept;
};

//----------------------------------------------------------------------------
//  Timing
//----------------------------------------------------------------------------
/// Monotonic elapsed-time clock.
class Clock final
{
public:
    /// Underlying steady clock type.
    using SteadyClock = std::chrono::steady_clock;
    /// Underlying duration type.
    using Duration = SteadyClock::duration;
    /// Underlying time point type.
    using TimePoint = SteadyClock::time_point;

    /// Starts a clock at the current time.
    Clock() noexcept;

    /// Resets the start time to now.
    void reset() noexcept;
    /// Returns elapsed duration since construction or last reset.
    [[nodiscard]] Duration elapsed() const noexcept;
    /// Returns elapsed seconds since construction or last reset.
    [[nodiscard]] double elapsedSeconds() const noexcept;
    /// Returns elapsed seconds and resets the start time to now.
    [[nodiscard]] double restartSeconds() noexcept;

private:
    TimePoint start_;
};

/// Per-frame timing result.
struct FrameTime
{
    /// Seconds since the previous frame tick.
    double deltaSeconds = 0.0;
    /// Seconds since the frame timer was created or reset.
    double totalSeconds = 0.0;
    /// Zero-based frame index.
    uint64_t frameIndex = 0;
};

/// Frame timer that leaves the application loop under caller control.
class FrameTimer final
{
public:
    /// Creates a frame timer starting at now.
    FrameTimer() noexcept;

    /// Resets elapsed time and frame index.
    void reset() noexcept;
    /// Advances and returns the next frame timing sample.
    [[nodiscard]] FrameTime tick() noexcept;

private:
    Clock clock_;
    double lastSeconds_ = 0.0;
    uint64_t frameIndex_ = 0;
};

/// Fixed-step simulation accumulator.
class FixedStepAccumulator final
{
public:
    /// Creates an accumulator with the requested fixed step duration.
    explicit FixedStepAccumulator(double fixedDeltaSeconds = 1.0 / 60.0) noexcept;

    /// Clears accumulated time.
    void reset() noexcept;
    /// Adds positive elapsed time to the accumulator.
    void add(double deltaSeconds) noexcept;
    /// Consumes one fixed step if enough time is accumulated.
    [[nodiscard]] bool consumeStep() noexcept;
    /// Returns interpolation alpha in fixed-step units.
    [[nodiscard]] double alpha() const noexcept;
    /// Returns accumulated seconds not yet consumed.
    [[nodiscard]] double accumulatedSeconds() const noexcept;
    /// Returns fixed step duration in seconds.
    [[nodiscard]] double fixedDeltaSeconds() const noexcept;

private:
    double fixedDeltaSeconds_ = 1.0 / 60.0;
    double accumulatedSeconds_ = 0.0;
};

/// Periodically samples frames per second from caller-provided frame deltas.
class FpsCounter final
{
public:
    /// Creates a counter that refreshes reported FPS after the update interval.
    explicit FpsCounter(double updateIntervalSeconds = 0.5) noexcept;

    /// Clears all accumulated and reported state.
    void reset() noexcept;
    /// Adds one frame delta and returns true when a new FPS sample is available.
    [[nodiscard]] bool update(double deltaSeconds) noexcept;
    /// Adds one frame timing sample and returns true when a new FPS sample is available.
    [[nodiscard]] bool update(const FrameTime& frameTime) noexcept;
    /// Returns the last sampled frames per second.
    [[nodiscard]] double framesPerSecond() const noexcept;
    /// Returns the last sampled average frame duration.
    [[nodiscard]] double frameSeconds() const noexcept;
    /// Returns the configured sampling interval in seconds.
    [[nodiscard]] double updateIntervalSeconds() const noexcept;
    /// Returns total frames passed to `update`.
    [[nodiscard]] uint64_t totalFrames() const noexcept;

private:
    double updateIntervalSeconds_ = 0.5;
    double accumulatedSeconds_ = 0.0;
    uint64_t accumulatedFrames_ = 0;
    uint64_t totalFrames_ = 0;
    double framesPerSecond_ = 0.0;
    double frameSeconds_ = 0.0;
};

/// Optional frame pacing helper that never owns the application loop.
class FrameLimiter final
{
public:
    /// Creates a limiter. A target FPS of zero disables sleep pacing.
    explicit FrameLimiter(double targetFramesPerSecond = 0.0) noexcept;

    /// Resets scheduling state.
    void reset() noexcept;
    /// Sets target FPS. Values less than or equal to zero disable sleep pacing.
    void setTargetFramesPerSecond(double framesPerSecond) noexcept;
    /// Disables sleep pacing.
    void clearTargetFramesPerSecond() noexcept;
    /// Returns target FPS, or zero when sleep pacing is disabled.
    [[nodiscard]] double targetFramesPerSecond() const noexcept;
    /// Returns target frame duration, or zero when sleep pacing is disabled.
    [[nodiscard]] double targetFrameSeconds() const noexcept;
    /// Records whether presentation is already paced by VSync.
    void setVSyncEnabled(bool enabled) noexcept;
    /// Returns whether VSync pacing is enabled.
    [[nodiscard]] bool isVSyncEnabled() const noexcept;
    /// Sleeps until the next target frame time unless disabled or VSync-paced.
    void wait() noexcept;

private:
    double targetFrameSeconds_ = 0.0;
    bool vSyncEnabled_ = false;
    bool started_ = false;
    Clock::TimePoint nextFrameTime_{};
};

//----------------------------------------------------------------------------
//  Events
//----------------------------------------------------------------------------
/// @cond CPPWINDOW_INTERNAL
namespace details {

template <typename T, typename Variant>
struct is_variant_member;

template <typename T, typename... Ts>
struct is_variant_member<T, std::variant<Ts...>> : std::bool_constant<(std::same_as<T, Ts> || ...)>
{};

}  // namespace details
/// @endcond

/// True when `T` is a payload type in `EventClass::Data`.
template <typename T, typename EventClass>
concept EventSubtypeOf = details::is_variant_member<T, typename EventClass::Data>::value;

/// Type-safe event variant for one window event.
class Event
{
public:
    /// Framebuffer pixel size changed.
    struct FramebufferResized
    {
        /// New framebuffer width in pixels.
        uint32_t width;
        /// New framebuffer height in pixels.
        uint32_t height;
    };

    /// Window close was requested by the platform.
    struct Closed
    {};

    /// Window content size changed.
    struct Resized
    {
        /// New window content width in screen coordinates.
        int width;
        /// New window content height in screen coordinates.
        int height;
    };

    /// Window position changed.
    struct Moved
    {
        /// New x position in virtual desktop coordinates.
        int x;
        /// New y position in virtual desktop coordinates.
        int y;
    };

    /// Window was minimized.
    struct Minimized
    {};

    /// Window was restored from minimized or maximized state.
    struct Restored
    {};

    /// Window was maximized.
    struct Maximized
    {};

    /// Window content scale changed.
    struct ContentScaleChanged
    {
        /// New horizontal content scale.
        float xScale;
        /// New vertical content scale.
        float yScale;
    };

    /// Window monitor association changed.
    struct MonitorChanged
    {
        /// New monitor id.
        uint32_t monitorId;
        /// Window mode at the time of the monitor change.
        WindowMode mode;
    };

    /// Window lost input focus.
    struct FocusLost
    {};

    /// Window gained input focus.
    struct FocusGained
    {};

    /// Unicode text input after keyboard layout/input method processing.
    struct TextEntered
    {
        /// Entered Unicode code point.
        char32_t unicode{};
    };

    /// Key press event.
    struct KeyPressed
    {
        /// Logical key.
        Key key{};
        /// Platform scancode.
        int scancode{};
        /// Modifier keys held for this event.
        Modifiers modifiers{};
    };

    /// Key release event.
    struct KeyReleased
    {
        /// Logical key.
        Key key{};
        /// Platform scancode.
        int scancode{};
        /// Modifier keys held for this event.
        Modifiers modifiers{};
    };

    /// Mouse wheel or trackpad scroll event.
    struct MouseWheelScrolled
    {
        /// Horizontal scroll delta.
        double deltaX, deltaY;
        /// Cursor x/y position in window coordinates.
        double posX, posY;
    };

    /// Mouse button press event.
    struct MouseButtonPressed
    {
        /// Mouse button.
        MouseButton button{};
        /// Cursor x/y position in window coordinates.
        double posX, posY;
        /// Modifier keys held for this event.
        Modifiers modifiers{};
    };

    /// Mouse button release event.
    struct MouseButtonReleased
    {
        /// Mouse button.
        MouseButton button{};
        /// Cursor x/y position in window coordinates.
        double posX, posY;
        /// Modifier keys held for this event.
        Modifiers modifiers{};
    };

    /// Mouse move event.
    struct MouseMoved
    {
        /// Cursor x/y position in window coordinates.
        double posX, posY;
    };

    /// Cursor entered the window.
    struct MouseEntered
    {};

    /// Cursor left the window.
    struct MouseLeft
    {};

    /// Standard gamepad connected.
    struct GamepadConnected
    {
        /// Gamepad slot id.
        uint32_t gamepadId{};
        /// Human-readable name when available.
        std::string name;
        /// Whether the device has a standard mapping.
        bool standardMapping{};
    };

    /// Standard gamepad disconnected.
    struct GamepadDisconnected
    {
        /// Gamepad slot id.
        uint32_t gamepadId{};
    };

    /// Standard gamepad button pressed.
    struct GamepadButtonPressed
    {
        /// Gamepad slot id.
        uint32_t gamepadId{};
        /// Pressed button.
        GamepadButton button{};
    };

    /// Standard gamepad button released.
    struct GamepadButtonReleased
    {
        /// Gamepad slot id.
        uint32_t gamepadId{};
        /// Released button.
        GamepadButton button{};
    };

    /// Standard gamepad axis moved.
    struct GamepadAxisMoved
    {
        /// Gamepad slot id.
        uint32_t gamepadId{};
        /// Moved axis.
        GamepadAxis axis{};
        /// Axis value.
        float value{};
    };

    /// Files dropped onto the window.
    struct FilesDropped
    {
        /// Dropped file paths.
        std::vector<std::string> paths;
        /// Cursor x position in window coordinates at drop time.
        double posX{};
        /// Cursor y position in window coordinates at drop time.
        double posY{};
    };

    /// Raw joystick button press event.
    struct JoystickButtonPressed
    {
        /// Joystick slot id.
        uint32_t joystickId{};
        /// Backend button index.
        uint32_t button{};
    };

    /// Raw joystick button release event.
    struct JoystickButtonReleased
    {
        /// Joystick slot id.
        uint32_t joystickId{};
        /// Backend button index.
        uint32_t button{};
    };

    /// Raw joystick axis movement event.
    struct JoystickMoved
    {
        /// Joystick slot id.
        uint32_t joystickId{};
        /// Backend axis index.
        uint32_t axis{};
        /// Backend axis position.
        float position{};
    };

    /// Raw joystick connection event.
    struct JoystickConnected
    {
        /// Joystick slot id.
        uint32_t joystickId{};
        /// Human-readable name when available.
        std::string name;
        /// Whether GLFW has a standard gamepad mapping for this joystick.
        bool standardMapping{};
        /// Number of raw axes reported by the backend.
        uint32_t axisCount{};
        /// Number of raw buttons reported by the backend.
        uint32_t buttonCount{};
    };

    /// Raw joystick disconnection event.
    struct JoystickDisconnected
    {
        /// Joystick slot id.
        uint32_t joystickId{};
    };

    /// Variant containing every event payload type.
    using Data = std::variant<
        FramebufferResized,
        Closed,
        Resized,
        Moved,
        Minimized,
        Restored,
        Maximized,
        ContentScaleChanged,
        MonitorChanged,
        FocusLost,
        FocusGained,
        TextEntered,
        KeyPressed,
        KeyReleased,
        MouseWheelScrolled,
        MouseButtonPressed,
        MouseButtonReleased,
        MouseMoved,
        MouseEntered,
        MouseLeft,
        GamepadConnected,
        GamepadDisconnected,
        GamepadButtonPressed,
        GamepadButtonReleased,
        GamepadAxisMoved,
        FilesDropped,
        JoystickButtonPressed,
        JoystickButtonReleased,
        JoystickMoved,
        JoystickConnected,
        JoystickDisconnected>;

    /// Creates a default event payload.
    Event() = default;

    /// Creates an event from any supported payload type.
    template <typename T>
        requires EventSubtypeOf<std::remove_cvref_t<T>, Event>
    Event(T&& eventData)
        : m_data(std::forward<T>(eventData))
    {
    }

    /// Returns whether this event currently stores payload type `T`.
    template <typename T>
        requires EventSubtypeOf<T, Event>
    [[nodiscard]] bool is() const
    {
        return std::holds_alternative<T>(m_data);
    }

    /// Returns a pointer to payload type `T`, or null when another type is stored.
    template <typename T>
        requires EventSubtypeOf<T, Event>
    [[nodiscard]] T* getIf()
    {
        return std::get_if<T>(&m_data);
    }

    /// Returns a const pointer to payload type `T`, or null when another type is stored.
    template <typename T>
        requires EventSubtypeOf<T, Event>
    [[nodiscard]] const T* getIf() const
    {
        return std::get_if<T>(&m_data);
    }

    /// Visits the stored payload with `std::visit`.
    template <typename Visitor>
    decltype(auto) visit(Visitor&& visitor) noexcept
    {
        return std::visit(std::forward<Visitor>(visitor), m_data);
    }

    /// Visits the stored payload with `std::visit`.
    template <typename Visitor>
    decltype(auto) visit(Visitor&& visitor) const noexcept
    {
        return std::visit(std::forward<Visitor>(visitor), m_data);
    }

private:
    Data m_data;
};

/// True when `Handler` can receive a payload of type `T` from `EventDispatcher`.
template <typename Handler, typename T>
concept EventPayloadHandlerFor =
    std::copy_constructible<std::decay_t<Handler>> &&
    ((std::invocable<std::decay_t<Handler>&, const T&> &&
      !std::invocable<std::decay_t<Handler>&, const Event&>) ||
     std::invocable<std::decay_t<Handler>&>);

/// True when `Handler` can receive raw events from `EventDispatcher`.
template <typename Handler>
concept EventHandlerFor =
    std::copy_constructible<std::decay_t<Handler>> &&
    (std::invocable<std::decay_t<Handler>&, const Event&> ||
     std::invocable<std::decay_t<Handler>&>);

/// Persistent typed event dispatcher.
///
/// Register handlers once, then call `dispatch()` with the current frame's
/// `Window::events()` span after `WindowContext::pollEvents()` or
/// `waitEvents*()`. `EventDispatcher` does not run the loop.
class EventDispatcher final
{
public:
    /// Opaque handle for a registered event handler.
    struct Subscription
    {
        /// Internal subscription id. Zero means no subscription.
        std::uint64_t id = 0;

        /// Returns whether this handle refers to a possible subscription.
        [[nodiscard]] explicit operator bool() const noexcept
        {
            return id != 0;
        }

        /// Compares two subscription handles.
        [[nodiscard]] friend bool operator==(Subscription, Subscription) = default;
    };

    /// Creates an empty dispatcher.
    EventDispatcher() = default;

    /// Dispatches every event to registered handlers.
    void dispatch(std::span<const Event> events) const
    {
        struct DispatchScope
        {
            explicit DispatchScope(const EventDispatcher& dispatcher) noexcept
                : dispatcher(dispatcher)
            {
                ++dispatcher.dispatchDepth_;
            }

            ~DispatchScope() noexcept
            {
                --dispatcher.dispatchDepth_;
                if (dispatcher.dispatchDepth_ == 0) {
                    dispatcher.compactDisconnectedHandlers();
                }
            }

            const EventDispatcher& dispatcher;
        };

        const DispatchScope scope{ *this };
        const std::size_t handlerCount = handlers_.size();

        for (const Event& event : events) {
            for (std::size_t index = 0; index < handlerCount; ++index) {
                if (!handlers_[index].connected) {
                    continue;
                }

                auto handler = handlers_[index].handler;
                handler(event);
            }
        }
    }

    /// Removes all registered handlers.
    void clear() noexcept
    {
        activeHandlerCount_ = 0;
        if (dispatchDepth_ == 0) {
            handlers_.clear();
            return;
        }

        for (HandlerEntry& entry : handlers_) {
            entry.connected = false;
        }
    }

    /// Returns whether no handlers are registered.
    [[nodiscard]] bool empty() const noexcept
    {
        return activeHandlerCount_ == 0;
    }

    /// Returns the number of registered handlers.
    [[nodiscard]] std::size_t handlerCount() const noexcept
    {
        return activeHandlerCount_;
    }

    /// Disconnects a registered handler. Returns true when a handler was removed.
    bool disconnect(Subscription subscription) noexcept
    {
        if (!subscription) {
            return false;
        }

        for (HandlerEntry& entry : handlers_) {
            if (entry.subscription == subscription && entry.connected) {
                entry.connected = false;
                --activeHandlerCount_;
                if (dispatchDepth_ == 0) {
                    compactDisconnectedHandlers();
                }
                return true;
            }
        }

        return false;
    }

    /// Returns whether a subscription is still connected.
    [[nodiscard]] bool isConnected(Subscription subscription) const noexcept
    {
        if (!subscription) {
            return false;
        }

        for (const HandlerEntry& entry : handlers_) {
            if (entry.subscription == subscription) {
                return entry.connected;
            }
        }

        return false;
    }

    /// Registers `handler` for every event payload of type `T` and returns its handle.
    ///
    /// The handler may accept `const T&` or no arguments.
    template <typename T, typename Handler>
        requires EventSubtypeOf<T, Event> && EventPayloadHandlerFor<Handler, T>
    [[nodiscard]] Subscription subscribe(Handler&& handler)
    {
        using StoredHandler = std::decay_t<Handler>;
        return addHandler(
            [handler = StoredHandler(std::forward<Handler>(handler))](const Event& event) mutable {
                if (const T* payload = event.getIf<T>()) {
                    invokeTyped<T>(handler, *payload);
                }
            });
    }

    /// Registers `handler` for every event payload of type `T`.
    ///
    /// The handler may accept `const T&` or no arguments.
    template <typename T, typename Handler>
        requires EventSubtypeOf<T, Event> && EventPayloadHandlerFor<Handler, T>
    EventDispatcher& on(Handler&& handler)
    {
        (void)subscribe<T>(std::forward<Handler>(handler));
        return *this;
    }

    /// Registers `handler` for every event and returns its handle.
    ///
    /// The handler may accept `const Event&` or no arguments.
    template <typename Handler>
        requires EventHandlerFor<Handler>
    [[nodiscard]] Subscription subscribeEach(Handler&& handler)
    {
        using StoredHandler = std::decay_t<Handler>;
        return addHandler(
            [handler = StoredHandler(std::forward<Handler>(handler))](const Event& event) mutable {
                if constexpr (std::invocable<StoredHandler&, const Event&>) {
                    handler(event);
                } else if constexpr (std::invocable<StoredHandler&>) {
                    handler();
                }
            });
    }

    /// Registers `handler` for every event.
    ///
    /// The handler may accept `const Event&` or no arguments.
    template <typename Handler>
        requires EventHandlerFor<Handler>
    EventDispatcher& each(Handler&& handler)
    {
        (void)subscribeEach(std::forward<Handler>(handler));
        return *this;
    }

private:
    struct HandlerEntry
    {
        Subscription subscription;
        std::function<void(const Event&)> handler;
        bool connected = true;
    };

    Subscription nextSubscription() noexcept
    {
        if (nextSubscriptionId_ == 0) {
            nextSubscriptionId_ = 1;
        }

        return Subscription{ nextSubscriptionId_++ };
    }

    void compactDisconnectedHandlers() const
    {
        std::erase_if(handlers_, [](const HandlerEntry& entry) {
            return !entry.connected;
        });
    }

    Subscription addHandler(std::function<void(const Event&)> handler)
    {
        const Subscription subscription = nextSubscription();
        handlers_.push_back(HandlerEntry{
            .subscription = subscription,
            .handler = std::move(handler),
            .connected = true,
        });
        ++activeHandlerCount_;
        return subscription;
    }

    template <typename T, typename Handler>
        requires EventPayloadHandlerFor<Handler, T>
    static void invokeTyped(Handler&& handler, const T& payload)
    {
        if constexpr (std::invocable<Handler&, const T&>) {
            handler(payload);
        } else if constexpr (std::invocable<Handler&>) {
            handler();
        }
    }

    mutable std::vector<HandlerEntry> handlers_;
    std::size_t activeHandlerCount_ = 0;
    std::uint64_t nextSubscriptionId_ = 1;
    mutable std::size_t dispatchDepth_ = 0;
};

//----------------------------------------------------------------------------
//  Input State
//----------------------------------------------------------------------------
/// Main window type.
class Window;
/// Backend-owned input snapshot storage.
struct InputStateData;

/// Snapshot-style input query interface for a window.
class InputState final
{
    friend class Window;

public:
    /// Returns whether a key is currently down.
    bool isKeyDown(Key key) const;
    /// Returns whether a key transitioned from up to down this frame.
    bool isKeyPressed(Key key) const;
    /// Returns whether a key transitioned from down to up this frame.
    bool isKeyReleased(Key key) const;

    /// Returns whether a mouse button is currently down.
    bool isMouseButtonDown(MouseButton button) const;
    /// Returns whether a mouse button transitioned from up to down this frame.
    bool isMouseButtonPressed(MouseButton button) const;
    /// Returns whether a mouse button transitioned from down to up this frame.
    bool isMouseButtonReleased(MouseButton button) const;
    /// Returns the current cursor position in window coordinates.
    std::pair<double, double> getMousePosition() const;
    /// Returns cursor movement since the previous poll/update.
    std::pair<double, double> getMouseDelta() const;
    /// Returns accumulated scroll delta since the previous poll/update.
    std::pair<double, double> getScrollDelta() const;
    /// Returns whether the cursor is inside the window content area.
    bool isMouseInside() const;

private:
    explicit InputState(const InputStateData* state);

    const InputStateData* state_;
};

//----------------------------------------------------------------------------
//  Input Helpers
//----------------------------------------------------------------------------
/// Keyboard key plus optional modifier requirements.
struct KeyBinding
{
    /// Keyboard key that triggers the action.
    Key key = Key::Unknown;
    /// Required modifiers. Empty modifiers mean plain key matching.
    Modifiers modifiers{};
    /// Additional keys that must be held, useful for side-specific modifiers.
    std::vector<Key> requiredKeys;
    /// When true, extra held modifiers prevent the binding from matching.
    bool exactModifiers = false;

    /// Compares two key bindings.
    [[nodiscard]] bool operator==(const KeyBinding&) const noexcept = default;
};

/// Direction filter for gamepad axis bindings.
enum class AxisDirection : int8_t
{
    /// Match either positive or negative axis movement.
    Any,
    /// Match only positive axis movement.
    Positive,
    /// Match only negative axis movement.
    Negative
};

/// Gamepad axis binding with a deadzone.
struct GamepadAxisBinding
{
    /// Standard gamepad axis that drives the action.
    GamepadAxis axis = GamepadAxis::LeftX;
    /// Absolute value below which the axis is ignored.
    float deadzone = 0.15f;
    /// Direction that activates the binding.
    AxisDirection direction = AxisDirection::Any;

    /// Compares two axis bindings.
    [[nodiscard]] bool operator==(const GamepadAxisBinding&) const noexcept = default;
};

/// Input sources that can trigger one named action.
struct ActionBinding
{
    /// Optional input context name. Empty means always enabled.
    std::string context;
    /// Keyboard keys that trigger the action, with optional modifiers.
    std::vector<KeyBinding> keys;
    /// Mouse buttons that trigger the action.
    std::vector<MouseButton> mouseButtons;
    /// Standard gamepad buttons that trigger the action.
    std::vector<GamepadButton> gamepadButtons;
    /// Standard gamepad axes that trigger or drive the action.
    std::vector<GamepadAxisBinding> gamepadAxes;
};

/// Stable handle for an action stored in an `ActionMap`.
struct ActionId
{
    /// Internal id. Zero means no action.
    uint32_t value = 0;

    /// Returns whether this handle refers to a possible action.
    [[nodiscard]] explicit operator bool() const noexcept
    {
        return value != 0;
    }

    /// Compares two action ids.
    [[nodiscard]] friend bool operator==(ActionId, ActionId) = default;
};

/// Optional action metadata for rebinding menus and debug UI.
struct ActionMetadata
{
    /// Human-readable label.
    std::string displayName;
    /// Longer UI/help description.
    std::string description;
};

/// Snapshot of one action and its current state.
struct ActionInfo
{
    /// Lightweight action id.
    ActionId id;
    /// Stable action name.
    std::string name;
    /// Optional UI metadata.
    ActionMetadata metadata;
    /// Current binding data.
    ActionBinding binding;
    /// Whether the action is currently down.
    bool down = false;
    /// Whether the action transitioned down on the last update.
    bool pressed = false;
    /// Whether the action transitioned up on the last update.
    bool released = false;
    /// Current axis value.
    float axisValue = 0.0f;
};

/// Small action binding map for game/app commands.
class ActionMap final
{
public:
    /// Returns an existing action id or creates one.
    [[nodiscard]] ActionId getOrCreateActionId(std::string action);
    /// Returns an action id by name, or an empty id when missing.
    [[nodiscard]] ActionId getActionId(std::string_view action) const noexcept;
    /// Returns whether an action exists.
    [[nodiscard]] bool hasAction(std::string_view action) const noexcept;
    /// Returns whether an action exists.
    [[nodiscard]] bool hasAction(ActionId action) const noexcept;
    /// Returns a snapshot of every action, suitable for debug UI or rebinding screens.
    [[nodiscard]] std::vector<ActionInfo> getActions() const;

    /// Sets metadata for an existing action id.
    ActionMap& setMetadata(ActionId action, ActionMetadata metadata);
    /// Returns metadata for an action id, or null when missing.
    [[nodiscard]] const ActionMetadata* getMetadata(ActionId action) const noexcept;

    /// Binds a key plus optional required modifiers to an action id.
    ActionMap&
    bindKey(ActionId action, Key key, Modifiers modifiers = {}, bool exactModifiers = false);
    /// Binds a key plus additional required held keys to an action id.
    ActionMap& bindKeyCombo(ActionId action, Key key, std::vector<Key> requiredKeys);
    /// Binds a mouse button to an action id.
    ActionMap& bindMouseButton(ActionId action, MouseButton button);
    /// Binds a standard gamepad button to an action id.
    ActionMap& bindGamepadButton(ActionId action, GamepadButton button);
    /// Binds a standard gamepad axis to an action id.
    ActionMap& bindGamepadAxis(
        ActionId action,
        GamepadAxis axis,
        float deadzone = 0.15f,
        AxisDirection direction = AxisDirection::Any);
    /// Replaces key bindings for an action id.
    ActionMap&
    replaceKey(ActionId action, Key key, Modifiers modifiers = {}, bool exactModifiers = false);
    /// Replaces key bindings with a key plus additional required held keys for an action id.
    ActionMap& replaceKeyCombo(ActionId action, Key key, std::vector<Key> requiredKeys);
    /// Replaces mouse button bindings for an action id.
    ActionMap& replaceMouseButton(ActionId action, MouseButton button);
    /// Replaces gamepad button bindings for an action id.
    ActionMap& replaceGamepadButton(ActionId action, GamepadButton button);
    /// Replaces gamepad axis bindings for an action id.
    ActionMap& replaceGamepadAxis(
        ActionId action,
        GamepadAxis axis,
        float deadzone = 0.15f,
        AxisDirection direction = AxisDirection::Any);
    /// Removes all input bindings for an action id while preserving context and state.
    void clearBindings(ActionId action);
    /// Removes all bindings and state for one action id.
    void clear(ActionId action);
    /// Removes every binding and action state.
    void clearAll();
    /// Clears action transition state but keeps bindings.
    void resetState() noexcept;
    /// Assigns an action id to an input context. Empty context means always enabled.
    ActionMap& setContext(ActionId action, std::string context);
    /// Enables or disables an input context.
    void setContextEnabled(std::string context, bool enabled);
    /// Returns whether an input context is enabled. Unknown contexts are enabled.
    [[nodiscard]] bool isContextEnabled(std::string_view context) const noexcept;
    /// Clears all explicit context enable/disable state.
    void clearContextStates();

    /// Updates action states from input and an optional gamepad snapshot.
    template <typename Input>
    void update(const Input& input, const std::optional<GamepadState>& gamepad = std::nullopt)
    {
        for (auto& entry : entries_) {
            entry.previousDown = entry.down;

            if (!isContextEnabled(entry.binding.context)) {
                entry.down = false;
                entry.axisValue = 0.0f;
                continue;
            }

            entry.axisValue = bindingAxisValue(entry.binding, gamepad);
            entry.down =
                isDigitalBindingDown(entry.binding, input, gamepad) || entry.axisValue != 0.0f;
        }
    }

    /// Returns whether an action is currently down.
    [[nodiscard]] bool isDown(ActionId action) const;
    /// Returns whether an action id transitioned from up to down on the last update.
    [[nodiscard]] bool isPressed(ActionId action) const;
    /// Returns whether an action id transitioned from down to up on the last update.
    [[nodiscard]] bool isReleased(ActionId action) const;
    /// Returns the current axis value for an action id, or zero when inactive.
    [[nodiscard]] float getAxis(ActionId action) const;
    /// Returns the binding for an action id, or null if it does not exist.
    [[nodiscard]] const ActionBinding* getBinding(ActionId action) const noexcept;

private:
    struct ContextState
    {
        std::string context;
        bool enabled = true;
    };

    struct Entry
    {
        ActionId id;
        std::string action;
        ActionMetadata metadata;
        ActionBinding binding;
        bool down = false;
        bool previousDown = false;
        float axisValue = 0.0f;
    };

    [[nodiscard]] Entry* findEntry(std::string_view action) noexcept;
    [[nodiscard]] const Entry* findEntry(std::string_view action) const noexcept;
    [[nodiscard]] Entry* findEntry(ActionId action) noexcept;
    [[nodiscard]] const Entry* findEntry(ActionId action) const noexcept;
    [[nodiscard]] ContextState* findContext(std::string_view context) noexcept;
    [[nodiscard]] const ContextState* findContext(std::string_view context) const noexcept;
    [[nodiscard]] Entry& getOrCreateEntry(std::string action);
    [[nodiscard]] ActionId nextActionId() noexcept;
    static void resetEntryState(Entry& entry) noexcept;

    template <typename Input>
    static bool isDigitalBindingDown(
        const ActionBinding& binding,
        const Input& input,
        const std::optional<GamepadState>& gamepad)
    {
        const Modifiers activeModifiers = currentModifiers(input);
        for (const KeyBinding& key : binding.keys) {
            if (input.isKeyDown(key.key) && requiredKeysDown(key.requiredKeys, input) &&
                modifiersMatch(key.modifiers, activeModifiers, key.exactModifiers)) {
                return true;
            }
        }

        for (MouseButton button : binding.mouseButtons) {
            if (input.isMouseButtonDown(button)) {
                return true;
            }
        }

        if (gamepad) {
            for (GamepadButton button : binding.gamepadButtons) {
                if (gamepad->isButtonDown(button)) {
                    return true;
                }
            }
        }

        return false;
    }

    template <typename Input>
    static bool requiredKeysDown(const std::vector<Key>& keys, const Input& input)
    {
        for (Key key : keys) {
            if (!input.isKeyDown(key)) {
                return false;
            }
        }

        return true;
    }

    template <typename Input>
    static Modifiers currentModifiers(const Input& input)
    {
        return Modifiers{
            .alt = input.isKeyDown(Key::LAlt) || input.isKeyDown(Key::RAlt),
            .control = input.isKeyDown(Key::LControl) || input.isKeyDown(Key::RControl),
            .shift = input.isKeyDown(Key::LShift) || input.isKeyDown(Key::RShift),
            .system = input.isKeyDown(Key::LSuper) || input.isKeyDown(Key::RSuper),
        };
    }

    static bool modifiersMatch(Modifiers required, Modifiers active, bool exact) noexcept
    {
        if (required.alt && !active.alt) {
            return false;
        }
        if (required.control && !active.control) {
            return false;
        }
        if (required.shift && !active.shift) {
            return false;
        }
        if (required.system && !active.system) {
            return false;
        }

        return !exact || required == active;
    }

    static float bindingAxisValue(
        const ActionBinding& binding,
        const std::optional<GamepadState>& gamepad) noexcept
    {
        if (!gamepad) {
            return 0.0f;
        }

        float result = 0.0f;
        for (const GamepadAxisBinding& axis : binding.gamepadAxes) {
            const float value = axisBindingValue(*gamepad, axis);
            if (std::abs(value) > std::abs(result)) {
                result = value;
            }
        }
        return result;
    }

    static float axisBindingValue(
        const GamepadState& gamepad,
        const GamepadAxisBinding& binding) noexcept
    {
        const float value = gamepad.getAxis(binding.axis);
        const float magnitude = std::abs(value);
        if (magnitude <= binding.deadzone) {
            return 0.0f;
        }

        switch (binding.direction) {
            case AxisDirection::Any:
                return value;
            case AxisDirection::Positive:
                return value > binding.deadzone ? value : 0.0f;
            case AxisDirection::Negative:
                return value < -binding.deadzone ? value : 0.0f;
        }

        return 0.0f;
    }

    std::vector<Entry> entries_;
    std::vector<ContextState> contexts_;
    uint32_t nextActionId_ = 1;
};

//----------------------------------------------------------------------------
//  Window
//----------------------------------------------------------------------------
/// Fluent builder for `Window`.
class WindowBuilder;

/// Native window wrapper with graphics, event, and input controls.
class Window final
{
    friend class WindowBuilder;

public:
    /// Destroys the native window.
    ~Window();
    /// Moves window ownership.
    Window(Window&&) noexcept;
    /// Windows are unique owners and cannot be copied.
    Window(const Window&) = delete;

    /// Windows are unique owners and cannot be copy-assigned.
    Window& operator=(const Window&) = delete;
    /// Move-assignment is disabled because input state references native storage.
    Window& operator=(Window&&) = delete;

    /// Returns platform-native handles for advanced integration.
    NativeHandles getNativeHandles() const;
    /// Creates a Vulkan surface for the given native VkInstance pointer.
    VulkanHandle createVulkanSurface(void* instance) const;
    /// Makes this window's OpenGL context current on the calling thread.
    void makeContextCurrent();
    /// Swaps the front and back buffers.
    void swapBuffers();

    /// Returns whether the window should close.
    bool shouldClose() const noexcept;
    /// Requests that the window close.
    void requestClose() noexcept;

    /// Returns read-only events collected by the last context poll.
    std::span<const Event> events() const noexcept;
    /// Returns the input query interface for this window.
    const InputState& getInput() const noexcept;

    /// Sets the window title.
    void setTitle(const std::string& title);
    /// Sets the content size in screen coordinates.
    void setSize(int width, int height);
    /// Sets the window position in virtual desktop coordinates.
    void setPosition(int x, int y);
    /// Applies optional min/max size limits.
    void setSizeLimits(const SizeLimits& limits);
    /// Removes size limits.
    void clearSizeLimits();
    /// Sets the content aspect ratio.
    void setAspectRatio(AspectRatio ratio);
    /// Removes the content aspect ratio constraint.
    void clearAspectRatio();
    /// Enables or disables user resizing.
    void setResizable(bool resizable);
    /// Enables or disables platform decorations such as title bar and border.
    void setDecorated(bool decorated);
    /// Enables or disables always-on-top behavior.
    void setFloating(bool floating);
    /// Sets window opacity in the platform-supported range.
    void setOpacity(float opacity);
    /// Enables or disables swap interval synchronization.
    void setVSync(bool enabled);
    /// Sets cursor visibility/capture mode.
    void setCursorMode(CursorMode mode);
    /// Sets a standard platform cursor shape. Returns false when unsupported.
    [[nodiscard]] bool setCursorShape(CursorShape shape);
    /// Sets a custom RGBA cursor image with hotspot coordinates. Returns false on invalid input.
    [[nodiscard]] bool setCursorImage(const ImageRgba& image, int hotX = 0, int hotY = 0);
    /// Restores the platform default cursor image.
    void clearCursor();
    /// Sets the cursor position in window coordinates.
    void setMousePosition(double x, double y);
    /// Enables or disables raw mouse motion when supported by the backend.
    [[nodiscard]] bool setRawMouseMotion(bool enabled);
    /// Minimizes the window.
    void minimize();
    /// Maximizes the window.
    void maximize();
    /// Restores the window from minimized, maximized, or fullscreen mode.
    void restore();
    /// Changes the presentation mode, optionally targeting a monitor/video mode.
    void setWindowMode(
        WindowMode mode,
        uint32_t monitorId = 0,
        std::optional<VideoMode> videoMode = std::nullopt);
    /// Sets one window icon image. Returns false when unsupported or invalid.
    [[nodiscard]] bool setIcon(const ImageRgba& image);
    /// Sets multiple window icon images. Returns false when unsupported or invalid.
    [[nodiscard]] bool setIcons(std::span<const ImageRgba> images);
    /// Restores the platform default window icon.
    void clearIcon();
    /// Requests user attention for the window.
    void requestAttention();
    /// Requests or clears input focus where the platform permits it.
    void setFocus(bool focus) const noexcept;
    /// Shows or hides the window.
    void setVisible(bool visible) const noexcept;
    /// Returns the content size in screen coordinates.
    std::pair<int, int> getSize() const noexcept;
    /// Returns the window position in virtual desktop coordinates.
    std::pair<int, int> getPosition() const noexcept;
    /// Returns framebuffer pixel size.
    std::pair<uint32_t, uint32_t> getFramebufferSize() const noexcept;
    /// Returns content scale for the window.
    std::pair<float, float> getContentScale() const noexcept;
    /// Returns DPI/content scale conversion helper for this window.
    DpiScale getDpiScale() const noexcept;
    /// Returns current window opacity.
    float getOpacity() const noexcept;
    /// Returns current cursor mode.
    CursorMode getCursorMode() const noexcept;
    /// Returns whether raw mouse motion is enabled for this window.
    bool isRawMouseMotionEnabled() const noexcept;
    /// Returns current presentation mode.
    WindowMode getWindowMode() const noexcept;
    /// Returns whether user resizing is enabled.
    bool isResizable() const noexcept;
    /// Returns whether platform decorations are enabled.
    bool isDecorated() const noexcept;
    /// Returns whether always-on-top behavior is enabled.
    bool isFloating() const noexcept;
    /// Returns whether the window is minimized/iconified.
    bool isMinimized() const noexcept;
    /// Returns whether the window is maximized.
    bool isMaximized() const noexcept;
    /// Returns whether the window has input focus.
    bool isFocused() const noexcept;
    /// Returns whether the window is visible.
    bool isVisible() const noexcept;

private:
    struct Impl;

    explicit Window(std::unique_ptr<Impl> impl);

    InputState inputState_;
    std::unique_ptr<Impl> impl_;
};

//----------------------------------------------------------------------------
//  Window Builder
//----------------------------------------------------------------------------
/// Fluent builder used to create a configured `Window`.
class WindowBuilder
{
public:
    /// Creates a builder with default title, size, visibility, and no graphics API.
    WindowBuilder();
    /// Destroys builder state.
    ~WindowBuilder();

    /// Sets the initial window title.
    WindowBuilder& title(std::string t);
    /// Sets the initial content size.
    WindowBuilder& size(int w, int h);
    /// Sets the initial window position in virtual desktop coordinates.
    WindowBuilder& position(int x, int y);
    /// Requests an OpenGL context with the given configuration.
    WindowBuilder& openGL(OpenGLConfig cfg = {});
    /// Requests no graphics API for this window.
    WindowBuilder& noGraphicsApi();
    /// Sets whether the window is initially visible.
    WindowBuilder& visible(bool visible = true);
    /// Creates the window hidden and unfocused.
    WindowBuilder& hidden();
    /// Sets whether the user can resize the window.
    WindowBuilder& resizable(bool resizable = true);
    /// Sets whether the window initially has input focus.
    WindowBuilder& focused(bool focused = true);
    /// Sets whether the window uses platform decorations.
    WindowBuilder& decorated(bool decorated = true);
    /// Creates the window without platform decorations.
    WindowBuilder& borderless();
    /// Sets whether the window is initially always-on-top.
    WindowBuilder& floating(bool floating = true);
    /// Sets initial platform opacity.
    WindowBuilder& opacity(float opacity);
    /// Sets initial min/max size limits.
    WindowBuilder& sizeLimits(const SizeLimits& limits);
    /// Sets initial aspect ratio.
    WindowBuilder& aspectRatio(AspectRatio ratio);
    /// Sets initial cursor visibility/capture mode.
    WindowBuilder& cursorMode(CursorMode mode);
    /// Sets initial raw mouse motion mode when supported by the backend.
    WindowBuilder& rawMouseMotion(bool enabled = true);
    /// Sets initial OpenGL swap interval behavior.
    WindowBuilder& vSync(bool enabled = true);
    /// Sets the initial presentation mode.
    WindowBuilder& windowMode(
        WindowMode mode,
        uint32_t monitorId = 0,
        std::optional<VideoMode> videoMode = std::nullopt);
    /// Creates the window.
    Window build();

private:
    struct Data;
    std::unique_ptr<Data> data_{};
};

//----------------------------------------------------------------------------
//  Window Context
//----------------------------------------------------------------------------
/// Process-wide windowing context and platform service entry point.
class WindowContext final
{
public:
    /// Returns the process-wide context singleton.
    static WindowContext& get();

    /// Destroys the windowing context after owned windows are gone.
    ~WindowContext();
    /// The context is a singleton and cannot be copied.
    WindowContext(const WindowContext&) = delete;
    /// The context is a singleton and cannot be assigned.
    WindowContext& operator=(const WindowContext&) = delete;

    /// Polls platform events and updates window/input state.
    void pollEvents() const noexcept;
    /// Waits until at least one platform event is available, then updates state.
    void waitEvents() const noexcept;
    /// Waits up to `timeoutSeconds` for events, then updates state.
    void waitEventsTimeout(double timeoutSeconds) const noexcept;
    /// Wakes a thread blocked in `waitEvents` or `waitEventsTimeout`.
    void postEmptyEvent() const noexcept;

    /// Returns a procedure loader for OpenGL or backend integration.
    ProcLoader getProcLoader() const;
    /// Returns whether Vulkan presentation support is available.
    bool isVulkanSupported() const;
    /// Returns Vulkan instance extensions required by the active window backend.
    std::vector<std::string> getRequiredVulkanInstanceExtensions() const;
    /// Returns metadata for all connected monitors.
    std::vector<MonitorInfo> getMonitors() const;
    /// Returns the primary monitor, if one is available.
    std::optional<MonitorInfo> getPrimaryMonitor() const;
    /// Returns video modes for a monitor id, defaulting to the primary monitor.
    std::vector<VideoMode> getVideoModes(uint32_t monitorId = 0) const;
    /// Returns monitor content scale, defaulting to the primary monitor.
    std::pair<float, float> getContentScale(uint32_t monitorId = 0) const;
    /// Returns DPI/content scale conversion helper for a monitor.
    DpiScale getDpiScale(uint32_t monitorId = 0) const;
    /// Returns connected standard gamepads.
    std::vector<GamepadInfo> getGamepads() const;
    /// Returns the current state for a standard gamepad.
    std::optional<GamepadState> getGamepadState(uint32_t gamepadId = 0) const;
    /// Returns whether raw mouse motion is supported by the backend/platform.
    bool isRawMouseMotionSupported() const;
    /// Sets the platform clipboard text. Returns false when the backend reports failure.
    [[nodiscard]] bool setClipboardText(const std::string& text) const;
    /// Returns whether clipboard text is currently available and non-empty.
    [[nodiscard]] bool hasClipboardText() const;
    /// Returns the platform clipboard text, or an empty string on failure.
    std::string getClipboardText() const;
    /// Returns platform clipboard text, or null when the backend reports failure.
    [[nodiscard]] std::optional<std::string> tryGetClipboardText() const;

private:
    struct Impl;

    WindowContext();

    std::unique_ptr<Impl> impl_{};
};

}  // namespace cwin

#endif
