/*
 * Copyright (c) 2026 Your Name
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#include "internal/InputMap.hpp"

#include "internal/GlfwIncludes.hpp"

namespace glfwvk {

namespace {

static constexpr StaticLookup<Key> KeyMap(
    {
        /* Printable keys */
        { Key::Space, GLFW_KEY_SPACE },
        { Key::Apostrophe, GLFW_KEY_APOSTROPHE },
        { Key::Comma, GLFW_KEY_COMMA },
        { Key::Minus, GLFW_KEY_MINUS },
        { Key::Period, GLFW_KEY_PERIOD },
        { Key::Slash, GLFW_KEY_SLASH },

        { Key::Num0, GLFW_KEY_0 },
        { Key::Num1, GLFW_KEY_1 },
        { Key::Num2, GLFW_KEY_2 },
        { Key::Num3, GLFW_KEY_3 },
        { Key::Num4, GLFW_KEY_4 },
        { Key::Num5, GLFW_KEY_5 },
        { Key::Num6, GLFW_KEY_6 },
        { Key::Num7, GLFW_KEY_7 },
        { Key::Num8, GLFW_KEY_8 },
        { Key::Num9, GLFW_KEY_9 },

        { Key::Semicolon, GLFW_KEY_SEMICOLON },
        { Key::Equal, GLFW_KEY_EQUAL },

        { Key::A, GLFW_KEY_A },
        { Key::B, GLFW_KEY_B },
        { Key::C, GLFW_KEY_C },
        { Key::D, GLFW_KEY_D },
        { Key::E, GLFW_KEY_E },
        { Key::F, GLFW_KEY_F },
        { Key::G, GLFW_KEY_G },
        { Key::H, GLFW_KEY_H },
        { Key::I, GLFW_KEY_I },
        { Key::J, GLFW_KEY_J },
        { Key::K, GLFW_KEY_K },
        { Key::L, GLFW_KEY_L },
        { Key::M, GLFW_KEY_M },
        { Key::N, GLFW_KEY_N },
        { Key::O, GLFW_KEY_O },
        { Key::P, GLFW_KEY_P },
        { Key::Q, GLFW_KEY_Q },
        { Key::R, GLFW_KEY_R },
        { Key::S, GLFW_KEY_S },
        { Key::T, GLFW_KEY_T },
        { Key::U, GLFW_KEY_U },
        { Key::V, GLFW_KEY_V },
        { Key::W, GLFW_KEY_W },
        { Key::X, GLFW_KEY_X },
        { Key::Y, GLFW_KEY_Y },
        { Key::Z, GLFW_KEY_Z },

        { Key::LBracket, GLFW_KEY_LEFT_BRACKET },
        { Key::Backslash, GLFW_KEY_BACKSLASH },
        { Key::RBracket, GLFW_KEY_RIGHT_BRACKET },
        { Key::Grave, GLFW_KEY_GRAVE_ACCENT },
        { Key::World1, GLFW_KEY_WORLD_1 },
        { Key::World2, GLFW_KEY_WORLD_2 },

        /* Function keys */
        { Key::Escape, GLFW_KEY_ESCAPE },
        { Key::Enter, GLFW_KEY_ENTER },
        { Key::Tab, GLFW_KEY_TAB },
        { Key::Backspace, GLFW_KEY_BACKSPACE },
        { Key::Insert, GLFW_KEY_INSERT },
        { Key::Delete, GLFW_KEY_DELETE },

        { Key::Right, GLFW_KEY_RIGHT },
        { Key::Left, GLFW_KEY_LEFT },
        { Key::Down, GLFW_KEY_DOWN },
        { Key::Up, GLFW_KEY_UP },

        { Key::PageUp, GLFW_KEY_PAGE_UP },
        { Key::PageDown, GLFW_KEY_PAGE_DOWN },
        { Key::Home, GLFW_KEY_HOME },
        { Key::End, GLFW_KEY_END },
        { Key::CapsLock, GLFW_KEY_CAPS_LOCK },
        { Key::ScrollLock, GLFW_KEY_SCROLL_LOCK },
        { Key::NumLock, GLFW_KEY_NUM_LOCK },
        { Key::PrintScreen, GLFW_KEY_PRINT_SCREEN },
        { Key::Pause, GLFW_KEY_PAUSE },

        /* Function keys */
        { Key::F1, GLFW_KEY_F1 },
        { Key::F2, GLFW_KEY_F2 },
        { Key::F3, GLFW_KEY_F3 },
        { Key::F4, GLFW_KEY_F4 },
        { Key::F5, GLFW_KEY_F5 },
        { Key::F6, GLFW_KEY_F6 },
        { Key::F7, GLFW_KEY_F7 },
        { Key::F8, GLFW_KEY_F8 },
        { Key::F9, GLFW_KEY_F9 },
        { Key::F10, GLFW_KEY_F10 },
        { Key::F11, GLFW_KEY_F11 },
        { Key::F12, GLFW_KEY_F12 },
        { Key::F13, GLFW_KEY_F13 },
        { Key::F14, GLFW_KEY_F14 },
        { Key::F15, GLFW_KEY_F15 },
        { Key::F16, GLFW_KEY_F16 },
        { Key::F17, GLFW_KEY_F17 },
        { Key::F18, GLFW_KEY_F18 },
        { Key::F19, GLFW_KEY_F19 },
        { Key::F20, GLFW_KEY_F20 },
        { Key::F21, GLFW_KEY_F21 },
        { Key::F22, GLFW_KEY_F22 },
        { Key::F23, GLFW_KEY_F23 },
        { Key::F24, GLFW_KEY_F24 },
        { Key::F25, GLFW_KEY_F25 },

        /* Numpad Keys */
        { Key::Numpad0, GLFW_KEY_KP_0 },
        { Key::Numpad1, GLFW_KEY_KP_1 },
        { Key::Numpad2, GLFW_KEY_KP_2 },
        { Key::Numpad3, GLFW_KEY_KP_3 },
        { Key::Numpad4, GLFW_KEY_KP_4 },
        { Key::Numpad5, GLFW_KEY_KP_5 },
        { Key::Numpad6, GLFW_KEY_KP_6 },
        { Key::Numpad7, GLFW_KEY_KP_7 },
        { Key::Numpad8, GLFW_KEY_KP_8 },
        { Key::Numpad9, GLFW_KEY_KP_9 },
        { Key::NumpadDecimal, GLFW_KEY_KP_DECIMAL },
        { Key::NumpadDivide, GLFW_KEY_KP_DIVIDE },
        { Key::NumpadMultiply, GLFW_KEY_KP_MULTIPLY },
        { Key::NumpadSubtract, GLFW_KEY_KP_SUBTRACT },
        { Key::NumpadAdd, GLFW_KEY_KP_ADD },
        { Key::NumpadEnter, GLFW_KEY_KP_ENTER },
        { Key::NumpadEqual, GLFW_KEY_KP_EQUAL },

        { Key::LShift, GLFW_KEY_LEFT_SHIFT },
        { Key::LControl, GLFW_KEY_LEFT_CONTROL },
        { Key::LAlt, GLFW_KEY_LEFT_ALT },
        { Key::LSuper, GLFW_KEY_LEFT_SUPER },
        { Key::RShift, GLFW_KEY_RIGHT_SHIFT },
        { Key::RControl, GLFW_KEY_RIGHT_CONTROL },
        { Key::RAlt, GLFW_KEY_RIGHT_ALT },
        { Key::RSuper, GLFW_KEY_RIGHT_SUPER },
        { Key::Menu, GLFW_KEY_MENU },
    });

static constexpr StaticLookup<MouseButton> MouseMap(
    {
        { MouseButton::Left, GLFW_MOUSE_BUTTON_LEFT },
        { MouseButton::Right, GLFW_MOUSE_BUTTON_RIGHT },
        { MouseButton::Middle, GLFW_MOUSE_BUTTON_MIDDLE },
        { MouseButton::Button4, GLFW_MOUSE_BUTTON_4 },
        { MouseButton::Button5, GLFW_MOUSE_BUTTON_5 },
        { MouseButton::Button6, GLFW_MOUSE_BUTTON_6 },
        { MouseButton::Button7, GLFW_MOUSE_BUTTON_7 },
        { MouseButton::Button8, GLFW_MOUSE_BUTTON_8 },
    });

}  // namespace

int InputMap::toGlfwKey(Key k)
{
    return KeyMap.toGlfw(k);
}

Key InputMap::toKey(int k)
{
    return (k >= 0 && k <= GLFW_KEY_LAST) ? KeyMap.toMapper(k) : Key::Unknown;
}

// Two-way Mouse
int InputMap::toGlfwMouseButton(MouseButton b)
{
    return MouseMap.toGlfw(b);
}

MouseButton InputMap::toMouseButton(int b)
{
    return (b >= 0 && b <= GLFW_MOUSE_BUTTON_LAST) ? MouseMap.toMapper(b) : MouseButton::Left;
}

}  // namespace glfwvk