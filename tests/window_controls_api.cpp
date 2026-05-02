#include <cppwindow/cppwindow.hpp>

#include <cassert>
#include <cstddef>
#include <type_traits>

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

    cwin::Event textEvent = cwin::Event::TextEntered{ .unicode = U'a' };
    assert(textEvent.is<cwin::Event::TextEntered>());
    assert(textEvent.getIf<cwin::Event::TextEntered>()->unicode == U'a');

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
}
