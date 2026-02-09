#pragma once

namespace glfwvk {

enum class Key : int
{
    Unknown = -1,
    Space = 0,
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
};

////////////////////////////////////////////////////////////
/// \brief The total number of keyboard keys, ignoring `Key::Unknown`
///
////////////////////////////////////////////////////////////
// NOLINTNEXTLINE(readability-identifier-naming)
inline constexpr unsigned int KeyCount{ static_cast<unsigned int>(Key::Menu) + 1 };

enum class Action : int
{
    Release = 0,
    Press,
    Repeat
};

enum class MouseButton : int
{
    Left = 0,  //!< The left mouse button
    Right,     //!< The right mouse button
    Middle,    //!< The middle (wheel) mouse button
    Button4,   //!< The first extra mouse button
    Button5,   //!< The second extra mouse button
    Button6,   //!< The third extra mouse button
    Button7,   //!< The fourth extra mouse button
    Button8,   //!< The fifth extra mouse button
};

static constexpr unsigned int MouseButtonCount{ 8 };

}  // namespace glfwvk
