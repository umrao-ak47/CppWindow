#include <cppwindow/cppwindow.hpp>

#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>

using namespace cwin;

namespace {

template <typename T>
constexpr const char* eventName()
{
    if constexpr (std::is_same_v<T, Event::FramebufferResized>) {
        return "FramebufferResized";
    } else if constexpr (std::is_same_v<T, Event::Closed>) {
        return "Closed";
    } else if constexpr (std::is_same_v<T, Event::Refresh>) {
        return "Refresh";
    } else if constexpr (std::is_same_v<T, Event::Resized>) {
        return "Resized";
    } else if constexpr (std::is_same_v<T, Event::Moved>) {
        return "Moved";
    } else if constexpr (std::is_same_v<T, Event::Minimized>) {
        return "Minimized";
    } else if constexpr (std::is_same_v<T, Event::Restored>) {
        return "Restored";
    } else if constexpr (std::is_same_v<T, Event::Maximized>) {
        return "Maximized";
    } else if constexpr (std::is_same_v<T, Event::ContentScaleChanged>) {
        return "ContentScaleChanged";
    } else if constexpr (std::is_same_v<T, Event::MonitorChanged>) {
        return "MonitorChanged";
    } else if constexpr (std::is_same_v<T, Event::FocusLost>) {
        return "FocusLost";
    } else if constexpr (std::is_same_v<T, Event::FocusGained>) {
        return "FocusGained";
    } else if constexpr (std::is_same_v<T, Event::TextEntered>) {
        return "TextEntered";
    } else if constexpr (std::is_same_v<T, Event::KeyPressed>) {
        return "KeyPressed";
    } else if constexpr (std::is_same_v<T, Event::KeyReleased>) {
        return "KeyReleased";
    } else if constexpr (std::is_same_v<T, Event::MouseWheelScrolled>) {
        return "MouseWheelScrolled";
    } else if constexpr (std::is_same_v<T, Event::MouseButtonPressed>) {
        return "MouseButtonPressed";
    } else if constexpr (std::is_same_v<T, Event::MouseButtonReleased>) {
        return "MouseButtonReleased";
    } else if constexpr (std::is_same_v<T, Event::MouseMoved>) {
        return "MouseMoved";
    } else if constexpr (std::is_same_v<T, Event::MouseEntered>) {
        return "MouseEntered";
    } else if constexpr (std::is_same_v<T, Event::MouseLeft>) {
        return "MouseLeft";
    } else if constexpr (std::is_same_v<T, Event::GamepadConnected>) {
        return "GamepadConnected";
    } else if constexpr (std::is_same_v<T, Event::GamepadDisconnected>) {
        return "GamepadDisconnected";
    } else if constexpr (std::is_same_v<T, Event::GamepadButtonPressed>) {
        return "GamepadButtonPressed";
    } else if constexpr (std::is_same_v<T, Event::GamepadButtonReleased>) {
        return "GamepadButtonReleased";
    } else if constexpr (std::is_same_v<T, Event::GamepadAxisMoved>) {
        return "GamepadAxisMoved";
    } else if constexpr (std::is_same_v<T, Event::FilesDropped>) {
        return "FilesDropped";
    } else if constexpr (std::is_same_v<T, Event::JoystickConnected>) {
        return "JoystickConnected";
    } else if constexpr (std::is_same_v<T, Event::JoystickDisconnected>) {
        return "JoystickDisconnected";
    } else if constexpr (std::is_same_v<T, Event::JoystickButtonPressed>) {
        return "JoystickButtonPressed";
    } else if constexpr (std::is_same_v<T, Event::JoystickButtonReleased>) {
        return "JoystickButtonReleased";
    } else if constexpr (std::is_same_v<T, Event::JoystickMoved>) {
        return "JoystickMoved";
    } else {
        return "UnknownEvent";
    }
}

template <typename Enum>
int enumIndex(Enum value)
{
    return static_cast<int>(value);
}

void logModifiers(const Modifiers& modifiers)
{
    std::cout << " alt=" << modifiers.alt << " control=" << modifiers.control
              << " shift=" << modifiers.shift << " system=" << modifiers.system;
}

void logEvent(const Event& event)
{
    event.visit([](const auto& payload) {
        using T = std::decay_t<decltype(payload)>;

        std::cout << eventName<T>();

        if constexpr (std::is_same_v<T, Event::FramebufferResized>) {
            std::cout << " width=" << payload.width << " height=" << payload.height;
        } else if constexpr (std::is_same_v<T, Event::Resized>) {
            std::cout << " width=" << payload.width << " height=" << payload.height;
        } else if constexpr (std::is_same_v<T, Event::Moved>) {
            std::cout << " x=" << payload.x << " y=" << payload.y;
        } else if constexpr (std::is_same_v<T, Event::ContentScaleChanged>) {
            std::cout << " xScale=" << payload.xScale << " yScale=" << payload.yScale;
        } else if constexpr (std::is_same_v<T, Event::MonitorChanged>) {
            std::cout << " monitor=" << payload.monitorId << " mode=" << enumIndex(payload.mode);
        } else if constexpr (std::is_same_v<T, Event::TextEntered>) {
            std::cout << " codepoint=" << static_cast<uint32_t>(payload.unicode);
        } else if constexpr (
            std::is_same_v<T, Event::KeyPressed> || std::is_same_v<T, Event::KeyReleased>) {
            auto& ctx = Context::get();
            const auto name = ctx.keyName(payload.key, payload.scancode);
            std::cout << " key=" << enumIndex(payload.key) << " scancode=" << payload.scancode;
            if (name) {
                std::cout << " name=" << *name;
            }
            logModifiers(payload.modifiers);
        } else if constexpr (std::is_same_v<T, Event::MouseWheelScrolled>) {
            std::cout << " delta=" << payload.deltaX << ", " << payload.deltaY
                      << " pos=" << payload.posX << ", " << payload.posY;
        } else if constexpr (
            std::is_same_v<T, Event::MouseButtonPressed> ||
            std::is_same_v<T, Event::MouseButtonReleased>) {
            std::cout << " button=" << enumIndex(payload.button) << " pos=" << payload.posX << ", "
                      << payload.posY;
            logModifiers(payload.modifiers);
        } else if constexpr (std::is_same_v<T, Event::MouseMoved>) {
            std::cout << " pos=" << payload.posX << ", " << payload.posY;
        } else if constexpr (std::is_same_v<T, Event::GamepadConnected>) {
            std::cout << " id=" << payload.gamepadId << " name=" << payload.name
                      << " standard=" << payload.standardMapping;
        } else if constexpr (std::is_same_v<T, Event::GamepadDisconnected>) {
            std::cout << " id=" << payload.gamepadId;
        } else if constexpr (
            std::is_same_v<T, Event::GamepadButtonPressed> ||
            std::is_same_v<T, Event::GamepadButtonReleased>) {
            std::cout << " id=" << payload.gamepadId << " button=" << enumIndex(payload.button);
        } else if constexpr (std::is_same_v<T, Event::GamepadAxisMoved>) {
            std::cout << " id=" << payload.gamepadId << " axis=" << enumIndex(payload.axis)
                      << " value=" << payload.value;
        } else if constexpr (std::is_same_v<T, Event::FilesDropped>) {
            std::cout << " files=" << payload.paths.size() << " pos=" << payload.posX << ", "
                      << payload.posY;
        } else if constexpr (std::is_same_v<T, Event::JoystickConnected>) {
            std::cout << " id=" << payload.joystickId << " name=" << payload.name
                      << " standard=" << payload.standardMapping << " axes=" << payload.axisCount
                      << " buttons=" << payload.buttonCount;
        } else if constexpr (std::is_same_v<T, Event::JoystickDisconnected>) {
            std::cout << " id=" << payload.joystickId;
        } else if constexpr (
            std::is_same_v<T, Event::JoystickButtonPressed> ||
            std::is_same_v<T, Event::JoystickButtonReleased>) {
            std::cout << " id=" << payload.joystickId << " button=" << payload.button;
        } else if constexpr (std::is_same_v<T, Event::JoystickMoved>) {
            std::cout << " id=" << payload.joystickId << " axis=" << payload.axis
                      << " position=" << payload.position;
        }

        std::cout << "\n";
    });
}

}  // namespace

int main()
{
    auto& ctx = Context::get();
    auto window =
        WindowBuilder{}.title("Event Viewer").size(900, 520).noGraphicsApi().resizable().build();

    std::cout << "Controls:\n"
              << "  Move window: log move events\n"
              << "  Resize window: log resize events\n"
              << "  Type text: log text events\n"
              << "  Click mouse: log button events\n"
              << "  Scroll mouse: log scroll events\n"
              << "  Drop files: log dropped paths\n"
              << "  Esc: close\n";

    uint64_t eventCount = 0;
    std::string lastEvent = "none";
    FrameLimiter frameLimiter(60.0);
    EventDispatcher dispatcher;
    dispatcher
        .each([&](const Event& event) {
            logEvent(event);
            ++eventCount;

            event.visit([&](const auto& payload) {
                using T = std::decay_t<decltype(payload)>;
                lastEvent = eventName<T>();
            });
        })
        .on<Event::Closed>([&] {
            window.requestClose();
        })
        .on<Event::KeyPressed>([&](const Event::KeyPressed& key) {
            if (key.key == Key::Escape) {
                window.requestClose();
            }
        });

    while (!window.shouldClose()) {
        ctx.pollEvents();
        dispatcher.dispatch(window.events());

        std::ostringstream title;
        title << "Event Viewer - " << eventCount << " events - last " << lastEvent;
        window.setTitle(title.str());

        frameLimiter.wait();
    }
}
