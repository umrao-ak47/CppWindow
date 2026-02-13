#include <cppwindow/cppwindow.hpp>

#include <iostream>

using namespace cwin;

int main()
{
    auto& ctx = WindowContext::Get();

    auto window = WindowBuilder{}.title("Basic Example").size(1280, 720).borderless().build();

    while (!window.shouldClose()) {
        ctx.pollEvents();

        // Use Escape to close window
        for (auto& e : window.events()) {
            if (auto event = e.getIf<Event::KeyPressed>()) {
                if (event->key == Key::Escape) {
                    std::cout << "Close window\n";
                    window.requestClose();
                }
            }
        }
    }
}
