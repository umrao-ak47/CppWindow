#include <cppwindow/cppwindow.hpp>

#include <cassert>
#include <cstddef>
#include <span>
#include <string>
#include <type_traits>
#include <utility>

static_assert(std::is_same_v<
              decltype(std::declval<const cwin::Window&>().events()),
              std::span<const cwin::Event>>);

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

    cwin::WindowBuilder builder;
    builder.title("Configured")
        .size(800, 600)
        .position(100, 120)
        .noAPI()
        .visible(false)
        .hidden()
        .focused(false)
        .resizable(false)
        .resizable()
        .decorated(false)
        .borderless()
        .floating()
        .opacity(0.8f)
        .sizeLimits(limits)
        .aspectRatio(ratio)
        .cursorMode(cwin::CursorMode::Hidden)
        .rawMouseMotion()
        .vSync(false)
        .windowMode(cwin::WindowMode::BorderlessFullscreen, monitor.id);

    cwin::Modifiers modifiers{
        .control = true,
        .shift = true,
    };
    assert(modifiers.any());
    assert(!modifiers.alt);
    assert(modifiers.control);
    assert(modifiers.shift);

    cwin::Error error(cwin::ErrorCode::BackendFailure, "backend failed");
    assert(error.code() == cwin::ErrorCode::BackendFailure);
    assert(std::string(error.what()) == "backend failed");

    cwin::GamepadInfo gamepadInfo{
        .id = 0,
        .name = "Standard Gamepad",
        .standardMapping = true,
    };
    cwin::GamepadState gamepadState{
        .id = gamepadInfo.id,
        .name = gamepadInfo.name,
        .standardMapping = gamepadInfo.standardMapping,
    };
    gamepadState.buttons[static_cast<std::size_t>(cwin::GamepadButton::A)] = true;
    gamepadState.axes[static_cast<std::size_t>(cwin::GamepadAxis::LeftX)] = 0.5f;

    assert(gamepadState.isButtonDown(cwin::GamepadButton::A));
    assert(!gamepadState.isButtonDown(cwin::GamepadButton::B));
    assert(gamepadState.getAxis(cwin::GamepadAxis::LeftX) == 0.5f);
    assert(gamepadState.getAxis(static_cast<cwin::GamepadAxis>(99)) == 0.0f);
    assert(cwin::MaxGamepads == 16);
    assert(cwin::MaxJoysticks == 16);

    cwin::Event textEvent = cwin::Event::TextEntered{ .unicode = U'a' };
    assert(textEvent.is<cwin::Event::TextEntered>());
    assert(textEvent.getIf<cwin::Event::TextEntered>()->unicode == U'a');

    cwin::Event shortcut = cwin::Event::KeyPressed{
        .key = cwin::Key::S,
        .scancode = 0,
        .modifiers = modifiers,
    };
    assert(shortcut.is<cwin::Event::KeyPressed>());
    assert(shortcut.getIf<cwin::Event::KeyPressed>()->modifiers.control);

    cwin::Event connected =
        cwin::Event::GamepadConnected{ .gamepadId = 2, .name = "Pad", .standardMapping = true };
    assert(connected.is<cwin::Event::GamepadConnected>());
    assert(connected.getIf<cwin::Event::GamepadConnected>()->gamepadId == 2);

    cwin::Event button = cwin::Event::GamepadButtonPressed{
        .gamepadId = 2,
        .button = cwin::GamepadButton::Start,
    };
    assert(button.is<cwin::Event::GamepadButtonPressed>());
    assert(button.getIf<cwin::Event::GamepadButtonPressed>()->button == cwin::GamepadButton::Start);

    cwin::Event axis = cwin::Event::GamepadAxisMoved{
        .gamepadId = 2,
        .axis = cwin::GamepadAxis::RightTrigger,
        .value = 1.0f,
    };
    axis.visit([](const auto& event) {
        using T = std::decay_t<decltype(event)>;
        if constexpr (std::is_same_v<T, cwin::Event::GamepadAxisMoved>) {
            assert(event.axis == cwin::GamepadAxis::RightTrigger);
            assert(event.value == 1.0f);
        }
    });

    cwin::Event joystickConnected = cwin::Event::JoystickConnected{
        .joystickId = 1,
        .name = "Raw Stick",
        .standardMapping = false,
        .axisCount = 4,
        .buttonCount = 12,
    };
    assert(joystickConnected.is<cwin::Event::JoystickConnected>());
    assert(joystickConnected.getIf<cwin::Event::JoystickConnected>()->axisCount == 4);

    cwin::Event joystickButton = cwin::Event::JoystickButtonPressed{
        .joystickId = 1,
        .button = 3,
    };
    assert(joystickButton.is<cwin::Event::JoystickButtonPressed>());
    assert(joystickButton.getIf<cwin::Event::JoystickButtonPressed>()->button == 3);

    cwin::Event joystickAxis = cwin::Event::JoystickMoved{
        .joystickId = 1,
        .axis = 2,
        .position = -0.5f,
    };
    assert(joystickAxis.is<cwin::Event::JoystickMoved>());
    assert(joystickAxis.getIf<cwin::Event::JoystickMoved>()->axis == 2);
    assert(joystickAxis.getIf<cwin::Event::JoystickMoved>()->position == -0.5f);

}
