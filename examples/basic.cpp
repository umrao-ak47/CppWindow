#include <cppwindow/cppwindow.hpp>

using namespace cppwindow;

int main()
{
    auto& ctx = WindowContext::Get();

    auto window = WindowBuilder{}.title("Basic Example").size(1280, 720).build();

    while (!window.shouldClose()) {
        ctx.pollEvents();

        for (auto& e : window.events()) {
            if (e.is<Event::Closed>()) {
                window.requestClose();
            }
        }
    }
}
