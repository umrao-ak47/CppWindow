#pragma once

#include <glfw-vk/KeyDefs.hpp>

#include <variant>
#include <vector>

namespace glfwvk {

namespace priv {

template <typename T, typename Variant>
struct is_variant_member;

template <typename T, typename... Ts>
struct is_variant_member<T, std::variant<Ts...>> : std::bool_constant<(std::same_as<T, Ts> || ...)>
{};

}  // namespace priv

template <typename T, typename EventClass>
concept EventSubtypeOf = priv::is_variant_member<T, typename EventClass::Data>::value;

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

private:
    Data m_data;
};

}  // namespace glfwvk
