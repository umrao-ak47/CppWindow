#include <cppwindow/cppwindow.hpp>
#ifdef CPPWINDOW_PACKAGE_CONSUMER_USE_IMGUI
#include <cppwindow/imgui.hpp>
#endif

#include <array>
#include <span>
#include <type_traits>
#include <utility>

#ifdef CPPWINDOW_PACKAGE_CONSUMER_USE_IMGUI
struct PackageRenderer {
    void newFrame();
    void render(ImDrawData*);
};
#endif

int main()
{
#ifdef CPPWINDOW_PACKAGE_CONSUMER_USE_IMGUI
    static_assert(std::is_constructible_v<cwin::imgui::Context>);
    static_assert(
        std::is_same_v<decltype(std::declval<cwin::imgui::Platform&>().newFrame()), void>);
    static_assert(cwin::imgui::Renderer<PackageRenderer>);
    static_assert(std::is_same_v<
                  decltype(std::declval<cwin::imgui::Layer<PackageRenderer>&>().render()),
                  void>);
#endif

    cwin::FrameLimiter limiter{ 60.0 };
    if (limiter.targetFramesPerSecond() != 60.0) {
        return 1;
    }

    cwin::ActionMap actions;
    const cwin::ActionId jump = actions.defineAction("jump");
    actions.setMetadata(jump, { "Jump", "Test installed action API" })
        .bindKey(jump, cwin::Key::Space);
    if (!jump || !actions.hasAction(jump) || actions.binding(jump) == nullptr) {
        return 1;
    }

    cwin::EventDispatcher dispatcher;
    int closedCount = 0;
    const auto subscription = dispatcher.subscribe<cwin::Event::Closed>([&] {
        ++closedCount;
    });

    const std::array<cwin::Event, 1> events{
        cwin::Event::Closed{},
    };
    dispatcher.dispatch(std::span<const cwin::Event>{ events.data(), events.size() });

    if (closedCount != 1 || !dispatcher.disconnect(subscription)) {
        return 1;
    }

    return 0;
}
