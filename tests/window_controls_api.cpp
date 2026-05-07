#include <cppwindow/cppwindow.hpp>

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <type_traits>
#include <utility>

static_assert(std::is_same_v<
              decltype(std::declval<const cwin::Window&>().events()),
              std::span<const cwin::Event>>);
static_assert(std::is_default_constructible_v<cwin::EventDispatcher>);
static_assert(std::is_default_constructible_v<cwin::EventDispatcher::Subscription>);
static_assert(std::is_same_v<
              decltype(std::declval<const cwin::EventDispatcher&>().dispatch(
                  std::declval<std::span<const cwin::Event>>())),
              void>);
static_assert(std::is_same_v<
              decltype(std::declval<cwin::EventDispatcher&>().disconnect(
                  std::declval<cwin::EventDispatcher::Subscription>())),
              bool>);
static_assert(std::is_same_v<
              decltype(std::declval<const cwin::EventDispatcher&>().isConnected(
                  std::declval<cwin::EventDispatcher::Subscription>())),
              bool>);
static_assert(
    std::is_same_v<decltype(std::declval<const cwin::Window&>().dpiScale()), cwin::DpiScale>);
static_assert(std::is_same_v<
              decltype(std::declval<const cwin::WindowContext&>().dpiScale()),
              cwin::DpiScale>);
static_assert(std::is_same_v<
              decltype(std::declval<cwin::Window&>().setCursorShape(cwin::CursorShape::Hand)),
              bool>);
static_assert(std::is_same_v<
              decltype(std::declval<cwin::Window&>()
                           .setCursorImage(std::declval<const cwin::ImageRgba&>(), 0, 0)),
              bool>);
static_assert(
    std::is_same_v<
        decltype(std::declval<cwin::Window&>().setIcon(std::declval<const cwin::ImageRgba&>())),
        bool>);
static_assert(std::is_same_v<
              decltype(std::declval<cwin::Window&>().setIcons(
                  std::declval<std::span<const cwin::ImageRgba>>())),
              bool>);
static_assert(std::is_same_v<decltype(std::declval<const cwin::Window&>().title()), std::string>);
static_assert(std::is_same_v<
              decltype(std::declval<const cwin::Window&>().windowedPlacement()),
              cwin::WindowPlacement>);
static_assert(std::is_same_v<
              decltype(std::declval<cwin::Window&>().setWindowedPlacement(
                  std::declval<const cwin::WindowPlacement&>())),
              void>);
static_assert(std::is_same_v<decltype(std::declval<cwin::Window&>().clearCursor()), void>);
static_assert(std::is_same_v<decltype(std::declval<cwin::Window&>().clearIcon()), void>);
static_assert(std::is_same_v<decltype(std::declval<cwin::Window&>().requestAttention()), void>);
static_assert(
    std::is_same_v<decltype(std::declval<const cwin::WindowContext&>().waitEvents()), void>);
static_assert(std::is_same_v<
              decltype(std::declval<const cwin::WindowContext&>().waitEventsTimeout(0.1)),
              void>);
static_assert(
    std::is_same_v<decltype(std::declval<const cwin::WindowContext&>().postEmptyEvent()), void>);
static_assert(std::is_same_v<
              decltype(std::declval<const cwin::WindowContext&>().setClipboardText(
                  std::declval<const std::string&>())),
              bool>);
static_assert(
    std::is_same_v<decltype(std::declval<const cwin::WindowContext&>().hasClipboardText()), bool>);
static_assert(std::is_same_v<
              decltype(std::declval<const cwin::WindowContext&>().clipboardText()),
              std::optional<std::string>>);
static_assert(std::is_same_v<
              decltype(std::declval<const cwin::WindowContext&>().keyName(cwin::Key::A)),
              std::optional<std::string>>);
static_assert(std::is_same_v<
              decltype(std::declval<const cwin::WindowContext&>().keyScancode(cwin::Key::A)),
              int>);
static_assert(std::is_same_v<decltype(std::declval<const cwin::Window&>().isResizable()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cwin::Window&>().isDecorated()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cwin::Window&>().isFloating()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cwin::Window&>().isMinimized()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cwin::Window&>().isMaximized()), bool>);

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
    cwin::WindowPlacement placement{
        .x = 100,
        .y = 120,
        .width = 800,
        .height = 600,
        .maximized = true,
    };

    assert(limits.minWidth == 320);
    assert(ratio.numerator == 16);
    assert(monitor.currentVideoMode.refreshRate == 60);
    assert(placement.width == 800);
    assert(placement.maximized);
    assert(cwin::CursorMode::Captured != cwin::CursorMode::Normal);
    assert(cwin::CursorShape::Hand != cwin::CursorShape::Arrow);
    assert(cwin::CursorShape::ResizeAll != cwin::CursorShape::NotAllowed);
    assert(cwin::WindowMode::BorderlessFullscreen != cwin::WindowMode::Windowed);
    assert(cwin::WindowMode::ExclusiveFullscreen != cwin::WindowMode::Fullscreen);

    std::array<uint8_t, 16> imagePixels{
        255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 255, 255, 255, 255,
    };
    cwin::ImageRgba image{
        .width = 2,
        .height = 2,
        .pixels = std::span<const uint8_t>{ imagePixels.data(), imagePixels.size() },
    };
    assert(image.width == 2);
    assert(image.height == 2);
    assert(image.pixels.size() == imagePixels.size());

    cwin::WindowBuilder builder;
    builder.title("Configured")
        .size(800, 600)
        .position(100, 120)
        .noGraphicsApi()
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
    assert(gamepadState.axis(cwin::GamepadAxis::LeftX) == 0.5f);
    assert(gamepadState.axis(static_cast<cwin::GamepadAxis>(99)) == 0.0f);
    assert(cwin::MaxGamepads == 16);
    assert(cwin::MaxJoysticks == 16);

    cwin::Event textEvent = cwin::Event::TextEntered{ .unicode = U'a' };
    assert(textEvent.is<cwin::Event::TextEntered>());
    assert(textEvent.getIf<cwin::Event::TextEntered>()->unicode == U'a');

    std::array<cwin::Event, 4> dispatchedEvents{
        cwin::Event::Closed{},
        cwin::Event::Refresh{},
        cwin::Event::KeyPressed{
            .key = cwin::Key::Escape,
            .scancode = 0,
            .modifiers = {},
        },
        cwin::Event::Resized{
            .width = 640,
            .height = 360,
        },
    };
    cwin::EventDispatcher dispatcher;
    assert(dispatcher.empty());
    int eachCount = 0;
    int closedCount = 0;
    bool escapeSeen = false;
    int resizedWidth = 0;
    cwin::EventDispatcher::Subscription eachSubscription =
        dispatcher.subscribeEach([&](const cwin::Event&) {
            ++eachCount;
        });
    cwin::EventDispatcher::Subscription closedSubscription =
        dispatcher.subscribe<cwin::Event::Closed>([&] {
            ++closedCount;
        });
    dispatcher.on<cwin::Event::KeyPressed>([&](const cwin::Event::KeyPressed& key) {
        escapeSeen = key.key == cwin::Key::Escape;
    });
    dispatcher.on<cwin::Event::Resized>([&](const cwin::Event::Resized& resized) {
        resizedWidth = resized.width;
    });
    assert(eachSubscription);
    assert(closedSubscription);
    assert(dispatcher.isConnected(eachSubscription));
    assert(dispatcher.isConnected(closedSubscription));
    assert(!dispatcher.isConnected({}));
    assert(!dispatcher.empty());
    assert(dispatcher.handlerCount() == 4);
    dispatcher.dispatch(
        std::span<const cwin::Event>{ dispatchedEvents.data(), dispatchedEvents.size() });
    assert(eachCount == 4);
    assert(closedCount == 1);
    assert(escapeSeen);
    assert(resizedWidth == 640);
    assert(dispatcher.disconnect(eachSubscription));
    assert(!dispatcher.disconnect(eachSubscription));
    assert(!dispatcher.disconnect({}));
    assert(!dispatcher.isConnected(eachSubscription));
    assert(dispatcher.handlerCount() == 3);
    escapeSeen = false;
    resizedWidth = 0;
    dispatcher.dispatch(
        std::span<const cwin::Event>{ dispatchedEvents.data(), dispatchedEvents.size() });
    assert(eachCount == 4);
    assert(closedCount == 2);
    assert(escapeSeen);
    assert(resizedWidth == 640);
    dispatcher.clear();
    assert(dispatcher.empty());
    assert(dispatcher.handlerCount() == 0);
    assert(!dispatcher.isConnected(closedSubscription));

    cwin::EventDispatcher disconnectingDispatcher;
    int firstHandlerCount = 0;
    int secondHandlerCount = 0;
    cwin::EventDispatcher::Subscription secondSubscription;
    const cwin::EventDispatcher::Subscription firstSubscription =
        disconnectingDispatcher.subscribeEach([&](const cwin::Event&) {
            ++firstHandlerCount;
            if (firstHandlerCount == 1) {
                assert(disconnectingDispatcher.disconnect(secondSubscription));
            } else {
                assert(!disconnectingDispatcher.isConnected(secondSubscription));
            }
        });
    secondSubscription = disconnectingDispatcher.subscribeEach([&](const cwin::Event&) {
        ++secondHandlerCount;
    });
    assert(firstSubscription);
    assert(secondSubscription);
    disconnectingDispatcher.dispatch(
        std::span<const cwin::Event>{ dispatchedEvents.data(), dispatchedEvents.size() });
    assert(firstHandlerCount == 4);
    assert(secondHandlerCount == 0);

    cwin::EventDispatcher clearingDispatcher;
    int afterClearCount = 0;
    const cwin::EventDispatcher::Subscription clearSubscription =
        clearingDispatcher.subscribeEach([&](const cwin::Event&) {
            clearingDispatcher.clear();
        });
    const cwin::EventDispatcher::Subscription afterClearSubscription =
        clearingDispatcher.subscribeEach([&](const cwin::Event&) {
            ++afterClearCount;
        });
    assert(clearSubscription);
    assert(afterClearSubscription);
    clearingDispatcher.dispatch(
        std::span<const cwin::Event>{ dispatchedEvents.data(), dispatchedEvents.size() });
    assert(afterClearCount == 0);
    assert(clearingDispatcher.empty());

    cwin::EventDispatcher chainingDispatcher;
    chainingDispatcher.each([&](const cwin::Event&) {
        ++eachCount;
    }).on<cwin::Event::Closed>([&] {
        ++closedCount;
    });
    assert(chainingDispatcher.handlerCount() == 2);

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
