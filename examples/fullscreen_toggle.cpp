#include <cppwindow/cppwindow.hpp>

#include <iostream>

using namespace cwin;

int main()
{
    auto& ctx = WindowContext::Get();

    auto window =
        WindowBuilder{}.title("Fullscreen Toggle").size(960, 540).noAPI().resizable().build();

    std::cout << "Keys: F fullscreen, B borderless fullscreen, W windowed, Esc close\n";

    while (!window.shouldClose()) {
        ctx.pollEvents();

        for (const auto& event : window.events()) {
            if (event.is<Event::Closed>()) {
                window.requestClose();
            }

            if (const auto* key = event.getIf<Event::KeyPressed>()) {
                if (key->key == Key::Escape) {
                    window.requestClose();
                } else if (key->key == Key::F) {
                    window.setWindowMode(WindowMode::Fullscreen);
                    std::cout << "fullscreen\n";
                } else if (key->key == Key::B) {
                    window.setWindowMode(WindowMode::BorderlessFullscreen);
                    std::cout << "borderless fullscreen\n";
                } else if (key->key == Key::W) {
                    window.setWindowMode(WindowMode::Windowed);
                    std::cout << "windowed\n";
                }
            }
        }
    }
}
