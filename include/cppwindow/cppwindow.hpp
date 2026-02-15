/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#ifndef CPPWINDOW_HEADER_CPPWINDOW_HPP
#define CPPWINDOW_HEADER_CPPWINDOW_HPP

#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <variant>

namespace cwin {

// This represents the actual function being loaded
using ProcFunction = void (*)();
// Proc loaders need a function pointer
using ProcLoader = ProcFunction (*)(const char*);

// In Vulkan, handles are 64-bit integers.
using VulkanHandle = uint64_t;

// Native Handles
struct NativeHandles
{
    enum class System
    {
        Win32,
        Cocoa,
        X11,
        WayLand,
        Unknown
    };

    System system = System::Unknown;
    void* window;
    void* display;
};

enum class GraphicsMode : uint8_t
{
    None,
    OpenGL
};

struct OpenGLConfig
{
    int major = 4;
    int minor = 5;
    bool coreProfile = true;
};

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

enum class Action : int
{
    Release = 0,
    Press,
    Repeat
};

enum class MouseButton : uint8_t
{
    Unknown = 0,
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

//  The total number of keyboard keys, ignoring `Key::Unknown`
inline constexpr unsigned int KeyCount{ static_cast<unsigned int>(Key::Last) };
// The total number of mouse buttons, ignoring `MouseButton::Unknown`
static constexpr unsigned int MouseButtonCount{ static_cast<unsigned int>(MouseButton::Last) };

//----------------------------------------------------------------------------
//  Events
//----------------------------------------------------------------------------
namespace details {

template <typename T, typename Variant>
struct is_variant_member;

template <typename T, typename... Ts>
struct is_variant_member<T, std::variant<Ts...>> : std::bool_constant<(std::same_as<T, Ts> || ...)>
{};

}  // namespace details

template <typename T, typename EventClass>
concept EventSubtypeOf = details::is_variant_member<T, typename EventClass::Data>::value;

class Event
{
public:
    struct FrameBufferResized
    {
        uint32_t width;
        uint32_t height;
    };

    struct Closed
    {};

    struct Resized
    {
        int width;
        int height;
    };

    struct FocusLost
    {};

    struct FocusGained
    {};

    struct TextEntered
    {
        char32_t unicode{};
    };

    struct KeyPressed
    {
        Key key{};
        int scancode{};
        bool alt{};
        bool control{};
        bool shift{};
        bool system{};
    };

    struct KeyReleased
    {
        Key key{};
        int scancode{};
        bool alt{};
        bool control{};
        bool shift{};
        bool system{};
    };

    struct MouseWheelScrolled
    {
        double deltaX, deltaY;
        double posX, posY;
    };

    struct MouseButtonPressed
    {
        MouseButton button{};
        double posX, posY;
        bool alt{};
        bool control{};
        bool shift{};
        bool system{};
    };

    struct MouseButtonReleased
    {
        MouseButton button{};
        double posX, posY;
        bool alt{};
        bool control{};
        bool shift{};
        bool system{};
    };

    struct MouseMoved
    {
        double posX, posY;
    };

    struct MouseEntered
    {};

    struct MouseLeft
    {};

    struct JoystickButtonPressed
    {
        unsigned int joystickId{};
        unsigned int button{};
    };

    struct JoystickButtonReleased
    {
        unsigned int joystickId{};
        unsigned int button{};
    };

    struct JoystickMoved
    {
        unsigned int joystickId{};
        // Joystick::Axis axis{};
        float position{};
    };

    struct JoystickConnected
    {
        unsigned int joystickId{};
    };

    struct JoystickDisconnected
    {
        unsigned int joystickId{};
    };

    struct TouchBegan
    {
        unsigned int finger{};
        // Vector2i position;
    };

    struct TouchMoved
    {
        unsigned int finger{};
        // Vector2i position;
    };

    struct TouchEnded
    {
        unsigned int finger{};
        // Vector2i position;
    };

    struct SensorChanged
    {
        // Sensor::Type type{};
        // Vector3f value;
    };

    using Data = std::variant<
        FrameBufferResized,
        Closed,
        Resized,
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
        JoystickButtonPressed,
        JoystickButtonReleased,
        JoystickMoved,
        JoystickConnected,
        JoystickDisconnected,
        TouchBegan,
        TouchMoved,
        TouchEnded,
        SensorChanged>;

    Event() = default;

    template <typename T>
        requires EventSubtypeOf<std::remove_cvref_t<T>, Event>
    Event(T&& eventData)
        : m_data(std::forward<T>(eventData))
    {
    }

    template <typename T>
        requires EventSubtypeOf<T, Event>
    [[nodiscard]] bool is() const
    {
        return std::holds_alternative<T>(m_data);
    }

    template <typename T>
        requires EventSubtypeOf<T, Event>
    [[nodiscard]] T* getIf()
    {
        return std::get_if<T>(&m_data);
    }

    template <typename T>
        requires EventSubtypeOf<T, Event>
    [[nodiscard]] const T* getIf() const
    {
        return std::get_if<T>(&m_data);
    }

    template <typename Visitor>
    decltype(auto) visit(Visitor&& visitor) noexcept
    {
        return std::visit(std::forward<Visitor>(visitor), m_data);
    }

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
class Window;
class NativeInputState;

class InputState final
{
    friend class Window;

public:
    // keyboard
    bool isKeyDown(Key key) const;
    bool isKeyPressed(Key key) const;
    bool isKeyReleased(Key key) const;

    // mouse
    bool isMouseButtonDown(MouseButton button) const;
    bool isMouseButtonPressed(MouseButton button) const;
    bool isMouseButtonReleased(MouseButton button) const;
    std::pair<double, double> getMousePosition() const;
    std::pair<double, double> getScrollDelta() const;

private:
    explicit InputState(const NativeInputState* state);

    const NativeInputState* state_;
};

//----------------------------------------------------------------------------
//  Window
//----------------------------------------------------------------------------
class NativeWindow;
class WindowBuilder;

class Window final
{
    friend class WindowBuilder;

public:
    ~Window();
    Window(Window&&) noexcept;
    Window(const Window&) = delete;

    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) = delete;

    NativeHandles getNativeHandles() const;
    VulkanHandle createVulkanSurface(void* instance) const;
    void makeContextCurrent();
    void swapBuffers();

    bool shouldClose() const noexcept;
    void requestClose() noexcept;

    std::span<Event> events() const noexcept;
    const InputState& getInput() const noexcept;

    void setTitle(const std::string& title);
    void setSize(int width, int height);
    void setFocus(bool focus) const noexcept;
    void setVisible(bool visible) const noexcept;
    std::pair<int, int> getSize() const noexcept;
    std::pair<uint32_t, uint32_t> getFrameBufferSize() const noexcept;
    bool isFocused() const noexcept;
    bool isVisible() const noexcept;

private:
    explicit Window(std::unique_ptr<NativeWindow> window);

    InputState inputState_;
    std::unique_ptr<NativeWindow> window_;
};

//----------------------------------------------------------------------------
//  Window Builder
//----------------------------------------------------------------------------
class WindowBuilder
{
public:
    WindowBuilder();
    ~WindowBuilder();

    WindowBuilder& title(std::string t);
    WindowBuilder& size(int w, int h);
    WindowBuilder& openGL(OpenGLConfig cfg = {});
    WindowBuilder& noAPI();
    WindowBuilder& hidden();
    WindowBuilder& resizable();
    WindowBuilder& borderless();
    Window build();

private:
    struct Data;
    std::unique_ptr<Data> data_{};
};

//----------------------------------------------------------------------------
//  Window Context
//----------------------------------------------------------------------------
class NativeWindowContext;

class WindowContext final
{
public:
    static WindowContext& Get();

    ~WindowContext();
    WindowContext(const WindowContext&) = delete;
    WindowContext& operator=(const WindowContext&) = delete;

    void pollEvents() const noexcept;

    ProcLoader getProcLoader() const;
    bool isVulkanSupported() const;
    std::vector<std::string> getRequiredGlfwVulkanExtensions() const;

private:
    WindowContext();

    std::unique_ptr<NativeWindowContext> context_{};
};

}  // namespace cwin

#endif