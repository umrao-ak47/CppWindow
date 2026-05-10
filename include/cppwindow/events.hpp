/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

/// @file events.hpp
/// @brief Window events and event dispatch helpers.

#ifndef CPPWINDOW_HEADER_EVENTS_HPP
#define CPPWINDOW_HEADER_EVENTS_HPP

#include <cppwindow/core.hpp>
#include <cppwindow/input.hpp>

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace cwin {

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

    /// Window contents should be redrawn.
    struct Refresh
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
        Refresh,
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
        handlers_.push_back(
            HandlerEntry{
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

}  // namespace cwin

#endif
