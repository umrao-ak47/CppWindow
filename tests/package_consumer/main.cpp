#include <cppwindow/cppwindow.hpp>

#include <array>
#include <span>

int main()
{
    cwin::FrameLimiter limiter{ 60.0 };
    if (limiter.targetFramesPerSecond() != 60.0) {
        return 1;
    }

    cwin::ActionMap actions;
    const cwin::ActionId jump = actions.getOrCreateActionId("jump");
    actions.setMetadata(jump, { "Jump", "Test installed action API" })
        .bindKey(jump, cwin::Key::Space);
    if (!jump || !actions.hasAction(jump) || actions.getBinding(jump) == nullptr) {
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
