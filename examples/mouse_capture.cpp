#include <cppwindow/cppwindow.hpp>

#include <iostream>
#include <sstream>

using namespace cwin;

int main()
{
    auto& ctx = WindowContext::Get();

    auto window = WindowBuilder{}.title("Mouse Capture").size(960, 540).noAPI().resizable().build();

    std::cout << "Keys: C toggle captured cursor, Esc close\n";

    bool captured = false;

    while (!window.shouldClose()) {
        ctx.pollEvents();

        for (const auto& event : window.events()) {
            if (event.is<Event::Closed>()) {
                window.requestClose();
            }

            if (const auto* key = event.getIf<Event::KeyPressed>()) {
                if (key->key == Key::Escape) {
                    window.requestClose();
                } else if (key->key == Key::C) {
                    captured = !captured;
                    window.setCursorMode(captured ? CursorMode::Captured : CursorMode::Normal);
                    std::cout << "captured: " << captured << "\n";
                }
            }

            if (const auto* mouse = event.getIf<Event::MouseMoved>()) {
                std::ostringstream title;
                title << "Mouse Capture - " << static_cast<int>(mouse->posX) << ", "
                      << static_cast<int>(mouse->posY);
                window.setTitle(title.str());
            }
        }
    }
}
