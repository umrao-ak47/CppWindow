#include <cppwindow/cppwindow.hpp>

#include <concepts>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

struct ClosedHandler
{
    void operator()(const cwin::Event::Closed&) const {}
};

struct RawEventHandler
{
    void operator()(const cwin::Event&) const {}
};

struct VoidHandler
{
    void operator()() const {}
};

struct BadHandler
{
    void operator()(int) const {}
};

struct MoveOnlyHandler
{
    MoveOnlyHandler() = default;
    MoveOnlyHandler(MoveOnlyHandler&&) = default;
    MoveOnlyHandler(const MoveOnlyHandler&) = delete;
    void operator()() const {}
};

struct CompileInput
{
    bool isKeyDown(cwin::Key) const
    {
        return false;
    }

    bool isMouseButtonDown(cwin::MouseButton) const
    {
        return false;
    }
};

template <typename Handler>
concept CanSubscribeClosed = requires(cwin::EventDispatcher dispatcher, Handler handler) {
    {
        dispatcher.subscribe<cwin::Event::Closed>(handler)
    } -> std::same_as<cwin::EventDispatcher::Subscription>;
};

template <typename Handler>
concept CanSubscribeEach = requires(cwin::EventDispatcher dispatcher, Handler handler) {
    { dispatcher.subscribeEach(handler) } -> std::same_as<cwin::EventDispatcher::Subscription>;
};

}  // namespace

static_assert(cwin::EventSubtypeOf<cwin::Event::Closed, cwin::Event>);
static_assert(cwin::EventSubtypeOf<cwin::Event::Refresh, cwin::Event>);
static_assert(cwin::EventSubtypeOf<cwin::Event::FramebufferResized, cwin::Event>);
static_assert(cwin::EventSubtypeOf<cwin::Event::KeyPressed, cwin::Event>);
static_assert(!cwin::EventSubtypeOf<int, cwin::Event>);

static_assert(cwin::EventPayloadHandlerFor<ClosedHandler, cwin::Event::Closed>);
static_assert(cwin::EventPayloadHandlerFor<VoidHandler, cwin::Event::Closed>);
static_assert(!cwin::EventPayloadHandlerFor<RawEventHandler, cwin::Event::Closed>);
static_assert(!cwin::EventPayloadHandlerFor<BadHandler, cwin::Event::Closed>);
static_assert(!cwin::EventPayloadHandlerFor<MoveOnlyHandler, cwin::Event::Closed>);

static_assert(cwin::EventHandlerFor<RawEventHandler>);
static_assert(cwin::EventHandlerFor<VoidHandler>);
static_assert(!cwin::EventHandlerFor<ClosedHandler>);
static_assert(!cwin::EventHandlerFor<BadHandler>);
static_assert(!cwin::EventHandlerFor<MoveOnlyHandler>);

static_assert(CanSubscribeClosed<ClosedHandler>);
static_assert(CanSubscribeClosed<VoidHandler>);
static_assert(!CanSubscribeClosed<RawEventHandler>);
static_assert(!CanSubscribeClosed<BadHandler>);
static_assert(!CanSubscribeClosed<MoveOnlyHandler>);

static_assert(CanSubscribeEach<RawEventHandler>);
static_assert(CanSubscribeEach<VoidHandler>);
static_assert(!CanSubscribeEach<ClosedHandler>);
static_assert(!CanSubscribeEach<BadHandler>);
static_assert(!CanSubscribeEach<MoveOnlyHandler>);

static_assert(std::is_same_v<
              decltype(std::declval<cwin::EventDispatcher&>().on<cwin::Event::Closed>(
                  std::declval<ClosedHandler>())),
              cwin::EventDispatcher&>);
static_assert(std::is_same_v<
              decltype(std::declval<cwin::EventDispatcher&>().each(
                  std::declval<RawEventHandler>())),
              cwin::EventDispatcher&>);

static_assert(std::is_same_v<decltype(cwin::WindowContext::get()), cwin::WindowContext&>);
static_assert(std::is_same_v<
              decltype(std::declval<const cwin::WindowContext&>().monitors()),
              std::vector<cwin::MonitorInfo>>);
static_assert(std::is_same_v<
              decltype(std::declval<const cwin::WindowContext&>()
                           .requiredVulkanInstanceExtensions()),
              std::vector<std::string>>);
static_assert(std::is_same_v<
              decltype(std::declval<const cwin::WindowContext&>().keyName(cwin::Key::A)),
              std::optional<std::string>>);
static_assert(std::is_same_v<
              decltype(std::declval<const cwin::WindowContext&>().keyName(
                  cwin::Key::Unknown,
                  42)),
              std::optional<std::string>>);
static_assert(std::is_same_v<
              decltype(std::declval<const cwin::WindowContext&>().keyScancode(cwin::Key::A)),
              int>);

static_assert(std::is_same_v<
              decltype(std::declval<cwin::ActionMap&>().defineAction("jump")),
              cwin::ActionId>);
static_assert(std::is_same_v<
              decltype(std::declval<const cwin::ActionMap&>().findAction(std::string_view{})),
              cwin::ActionId>);
static_assert(std::is_same_v<
              decltype(std::declval<const cwin::ActionMap&>().actions()),
              std::vector<cwin::ActionInfo>>);
static_assert(std::is_same_v<
              decltype(std::declval<cwin::ActionMap&>().setMetadata(
                  cwin::ActionId{ 1 },
                  cwin::ActionMetadata{})),
              cwin::ActionMap&>);
static_assert(std::is_same_v<
              decltype(std::declval<cwin::ActionMap&>().bindKey(
                  cwin::ActionId{ 1 },
                  cwin::Key::Space)),
              cwin::ActionMap&>);
static_assert(std::is_same_v<
              decltype(std::declval<cwin::ActionMap&>().bindKeyCombo(
                  cwin::ActionId{ 1 },
                  cwin::Key::A,
                  std::declval<std::vector<cwin::Key>>())),
              cwin::ActionMap&>);
static_assert(std::is_same_v<
              decltype(std::declval<cwin::ActionMap&>().replaceGamepadAxis(
                  cwin::ActionId{ 1 },
                  cwin::GamepadAxis::RightX,
                  0.2f,
                  cwin::AxisDirection::Negative)),
              cwin::ActionMap&>);
static_assert(std::is_same_v<
              decltype(std::declval<cwin::ActionMap&>().bindGamepadAxis(
                  cwin::ActionId{ 1 },
                  cwin::GamepadAxis::RightX,
                  0.2f,
                  cwin::AxisDirection::Positive)),
              cwin::ActionMap&>);
static_assert(std::is_same_v<
              decltype(std::declval<const cwin::ActionMap&>().binding(cwin::ActionId{ 1 })),
              const cwin::ActionBinding*>);
static_assert(std::is_same_v<
              decltype(std::declval<const cwin::ActionMap&>().isPressed(cwin::ActionId{ 1 })),
              bool>);
static_assert(requires(
    cwin::ActionMap actions,
    CompileInput input,
    std::optional<cwin::GamepadState> gamepad) {
    actions.update(input);
    actions.update(input, gamepad);
});

static_assert(std::is_same_v<
              decltype(std::declval<cwin::WindowBuilder&>()
                           .size(640, 480)
                           .title("app")
                           .noGraphicsApi()),
              cwin::WindowBuilder&>);
static_assert(std::is_same_v<
              decltype(std::declval<cwin::WindowBuilder&>().windowMode(
                  cwin::WindowMode::BorderlessFullscreen,
                  0,
                  std::optional<cwin::VideoMode>{})),
              cwin::WindowBuilder&>);
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
static_assert(std::is_same_v<
              decltype(std::declval<const cwin::Window&>().framebufferSize()),
              std::pair<uint32_t, uint32_t>>);
static_assert(std::is_same_v<decltype(std::declval<const cwin::Window&>().isResizable()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cwin::Window&>().isDecorated()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cwin::Window&>().isFloating()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cwin::Window&>().isMinimized()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cwin::Window&>().isMaximized()), bool>);

int main()
{
    return 0;
}
