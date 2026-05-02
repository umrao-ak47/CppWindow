#include <cppwindow/cppwindow.hpp>

#include <cstddef>
#include <iostream>

using namespace cwin;

int main()
{
    auto& ctx = WindowContext::Get();

    auto window = WindowBuilder{}.title("Basic Example").size(1280, 720).build();
    EventDispatcher dispatcher;
    dispatcher.on<Event::Closed>([&] {
        window.requestClose();
    });

    while (!window.shouldClose()) {
        ctx.waitEventsTimeout(1.0 / 60.0);

        if (const std::size_t queueSize = window.events().size(); queueSize != 0) {
            std::cout << "Event Queue Size: " << queueSize << "\n";
        }
        dispatcher.dispatch(window.events());
    }
}
