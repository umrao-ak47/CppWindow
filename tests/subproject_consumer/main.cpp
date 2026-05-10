#include <cppwindow/cppwindow.hpp>

#include <array>
#include <span>
#include <type_traits>
#include <utility>

int main()
{
    static_assert(std::is_same_v<decltype(cwin::Context::get()), cwin::Context&>);
    static_assert(std::is_same_v<
                  decltype(std::declval<const cwin::Window&>().events()),
                  std::span<const cwin::Event>>);

    cwin::FrameTimer timer;
    const cwin::FrameTime firstFrame = timer.tick();
    if (firstFrame.frameIndex != 0) {
        return 1;
    }

    cwin::EventDispatcher dispatcher;
    int closedCount = 0;
    dispatcher.on<cwin::Event::Closed>([&] {
        ++closedCount;
    });

    const std::array<cwin::Event, 1> events{
        cwin::Event::Closed{},
    };
    dispatcher.dispatch(std::span<const cwin::Event>{ events.data(), events.size() });

    cwin::ActionMap actions;
    const cwin::ActionId quit = actions.defineAction("quit");
    actions.bindKey(quit, cwin::Key::Escape);

    return closedCount == 1 && actions.hasAction(quit) ? 0 : 1;
}
