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
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
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
        /// Compatibility alias for Wayland.
        WayLand = Wayland,
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
    struct FrameBufferResized
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
        FrameBufferResized,
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

//----------------------------------------------------------------------------
//  Input State
//----------------------------------------------------------------------------
/// Main window type.
class Window;
/// Backend-owned input storage.
class NativeInputState;

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
    explicit InputState(const NativeInputState* state);

    const NativeInputState* state_;
};

//----------------------------------------------------------------------------
//  Input Helpers
//----------------------------------------------------------------------------
/// Input sources that can trigger one named action.
struct ActionBinding
{
    /// Keyboard keys that trigger the action.
    std::vector<Key> keys;
    /// Mouse buttons that trigger the action.
    std::vector<MouseButton> mouseButtons;
    /// Standard gamepad buttons that trigger the action.
    std::vector<GamepadButton> gamepadButtons;
};

/// Small action binding map for game/app commands.
class ActionMap final
{
public:
    /// Binds a key to an action name.
    ActionMap& bindKey(std::string action, Key key);
    /// Binds a mouse button to an action name.
    ActionMap& bindMouseButton(std::string action, MouseButton button);
    /// Binds a standard gamepad button to an action name.
    ActionMap& bindGamepadButton(std::string action, GamepadButton button);
    /// Removes all bindings and state for one action.
    void clear(const std::string& action);
    /// Removes every binding and action state.
    void clearAll();
    /// Clears action transition state but keeps bindings.
    void resetState() noexcept;

    /// Updates action states from input and an optional gamepad snapshot.
    template <typename Input>
    void update(const Input& input, const std::optional<GamepadState>& gamepad = std::nullopt)
    {
        for (auto& entry : entries_) {
            entry.previousDown = entry.down;
            entry.down = isBindingDown(entry.binding, input, gamepad);
        }
    }

    /// Returns whether an action is currently down.
    [[nodiscard]] bool isDown(const std::string& action) const;
    /// Returns whether an action transitioned from up to down on the last update.
    [[nodiscard]] bool isPressed(const std::string& action) const;
    /// Returns whether an action transitioned from down to up on the last update.
    [[nodiscard]] bool isReleased(const std::string& action) const;
    /// Returns the binding for an action, or null if it does not exist.
    [[nodiscard]] const ActionBinding* getBinding(const std::string& action) const noexcept;

private:
    struct Entry
    {
        std::string action;
        ActionBinding binding;
        bool down = false;
        bool previousDown = false;
    };

    [[nodiscard]] Entry* findEntry(const std::string& action) noexcept;
    [[nodiscard]] const Entry* findEntry(const std::string& action) const noexcept;
    [[nodiscard]] Entry& getOrCreateEntry(std::string action);

    template <typename Input>
    static bool isBindingDown(
        const ActionBinding& binding,
        const Input& input,
        const std::optional<GamepadState>& gamepad)
    {
        for (Key key : binding.keys) {
            if (input.isKeyDown(key)) {
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

    std::vector<Entry> entries_;
};

//----------------------------------------------------------------------------
//  Window
//----------------------------------------------------------------------------
/// Backend-owned window implementation.
class NativeWindow;
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
    /// Requests or clears input focus where the platform permits it.
    void setFocus(bool focus) const noexcept;
    /// Shows or hides the window.
    void setVisible(bool visible) const noexcept;
    /// Returns the content size in screen coordinates.
    std::pair<int, int> getSize() const noexcept;
    /// Returns the window position in virtual desktop coordinates.
    std::pair<int, int> getPosition() const noexcept;
    /// Returns framebuffer pixel size.
    std::pair<uint32_t, uint32_t> getFrameBufferSize() const noexcept;
    /// Returns content scale for the window.
    std::pair<float, float> getContentScale() const noexcept;
    /// Returns current window opacity.
    float getOpacity() const noexcept;
    /// Returns current cursor mode.
    CursorMode getCursorMode() const noexcept;
    /// Returns whether raw mouse motion is enabled for this window.
    bool isRawMouseMotionEnabled() const noexcept;
    /// Returns current presentation mode.
    WindowMode getWindowMode() const noexcept;
    /// Returns whether the window has input focus.
    bool isFocused() const noexcept;
    /// Returns whether the window is visible.
    bool isVisible() const noexcept;

private:
    explicit Window(std::unique_ptr<NativeWindow> window);

    InputState inputState_;
    std::unique_ptr<NativeWindow> window_;
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
    WindowBuilder& noAPI();
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
/// Backend-owned global windowing context.
class NativeWindowContext;

/// Process-wide windowing context and platform service entry point.
class WindowContext final
{
public:
    /// Returns the process-wide context singleton.
    static WindowContext& Get();

    /// Destroys the windowing context after owned windows are gone.
    ~WindowContext();
    /// The context is a singleton and cannot be copied.
    WindowContext(const WindowContext&) = delete;
    /// The context is a singleton and cannot be assigned.
    WindowContext& operator=(const WindowContext&) = delete;

    /// Polls platform events and updates window/input state.
    void pollEvents() const noexcept;

    /// Returns a procedure loader for OpenGL or backend integration.
    ProcLoader getProcLoader() const;
    /// Returns whether Vulkan presentation support is available.
    bool isVulkanSupported() const;
    /// Returns Vulkan instance extensions required by GLFW surface creation.
    std::vector<std::string> getRequiredGlfwVulkanExtensions() const;
    /// Returns metadata for all connected monitors.
    std::vector<MonitorInfo> getMonitors() const;
    /// Returns the primary monitor, if one is available.
    std::optional<MonitorInfo> getPrimaryMonitor() const;
    /// Returns video modes for a monitor id, defaulting to the primary monitor.
    std::vector<VideoMode> getVideoModes(uint32_t monitorId = 0) const;
    /// Returns monitor content scale, defaulting to the primary monitor.
    std::pair<float, float> getContentScale(uint32_t monitorId = 0) const;
    /// Returns connected standard gamepads.
    std::vector<GamepadInfo> getGamepads() const;
    /// Returns the current state for a standard gamepad.
    std::optional<GamepadState> getGamepadState(uint32_t gamepadId = 0) const;
    /// Returns whether raw mouse motion is supported by the backend/platform.
    bool isRawMouseMotionSupported() const;
    /// Sets the platform clipboard text.
    void setClipboardText(const std::string& text) const;
    /// Returns the platform clipboard text.
    std::string getClipboardText() const;

private:
    WindowContext();

    std::unique_ptr<NativeWindowContext> context_{};
};

}  // namespace cwin

#endif
