#include <cppwindow/cppwindow.hpp>

#include <iostream>
#include <sstream>
#include <string_view>

using namespace cwin;

namespace {

void handleWindow(Window& window, std::string_view name, int& presses)
{
    for (const auto& event : window.events()) {
        if (event.is<Event::Closed>()) {
            std::cout << name << " close requested\n";
            window.setVisible(false);
            window.requestClose();
        }

        if (const auto* key = event.getIf<Event::KeyPressed>()) {
            if (key->key == Key::Escape) {
                std::cout << name << " closed with Escape\n";
                window.setVisible(false);
                window.requestClose();
            } else if (key->key == Key::Space) {
                ++presses;
            }
        }

        if (const auto* resized = event.getIf<Event::Resized>()) {
            std::cout << name << " resized to " << resized->width << "x" << resized->height << "\n";
        }
    }

    if (!window.shouldClose()) {
        auto [x, y] = window.getInput().getMousePosition();
        std::ostringstream title;
        title << name << " - Space presses " << presses << " - mouse " << static_cast<int>(x)
              << ", " << static_cast<int>(y);
        window.setTitle(title.str());
    }
}

}  // namespace

int main()
{
    auto& ctx = WindowContext::Get();

    auto left = WindowBuilder{}.title("Left Window").size(560, 360).noAPI().resizable().build();
    auto right = WindowBuilder{}.title("Right Window").size(560, 360).noAPI().resizable().build();

    left.setPosition(120, 160);
    right.setPosition(720, 160);

    int leftPresses = 0;
    int rightPresses = 0;
    FrameLimiter frameLimiter(120.0);

    std::cout << "Controls:\n"
              << "  Space: count a press in the focused window\n"
              << "  Esc: close the focused window\n";

    while (!left.shouldClose() || !right.shouldClose()) {
        ctx.pollEvents();

        handleWindow(left, "Left", leftPresses);
        handleWindow(right, "Right", rightPresses);

        frameLimiter.wait();
    }
}
