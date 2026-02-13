#include <cppwindow/cppwindow.hpp>

#include <iostream>

using namespace cppwindow;

int main()
{
    auto& ctx = WindowContext::Get();

    auto window = WindowBuilder{}.title("Basic Example").size(1280, 720).build();

    while (!window.shouldClose()) {
        ctx.pollEvents();

        size_t queueSize = window.events().size();
        if (queueSize != 0) {
            std::cout << "Event Queue Size: " << queueSize << "\n";
        }
        for (auto& e : window.events()) {
            if (e.is<Event::Closed>()) {
                window.requestClose();
            }
        }
    }
}
