#include <cppwindow/cppwindow.hpp>

#include <iostream>
#include <sstream>
#include <string_view>

using namespace cwin;

namespace {

EventDispatcher makeWindowDispatcher(Window& window, std::string_view name, int& presses)
{
    EventDispatcher dispatcher;
    dispatcher
        .on<Event::Closed>([&, name] {
            std::cout << name << " close requested\n";
            window.setVisible(false);
            window.requestClose();
        })
        .on<Event::KeyPressed>([&, name](const Event::KeyPressed& key) {
            if (key.key == Key::Escape) {
                std::cout << name << " closed with Escape\n";
                window.setVisible(false);
                window.requestClose();
            } else if (key.key == Key::Space) {
                ++presses;
            }
        })
        .on<Event::Resized>([name](const Event::Resized& resized) {
            std::cout << name << " resized to " << resized.width << "x" << resized.height << "\n";
        });
    return dispatcher;
}

void updateWindowTitle(Window& window, std::string_view name, int presses)
{
    if (!window.shouldClose()) {
        auto [x, y] = window.input().mousePosition();
        std::ostringstream title;
        title << name << " - Space presses " << presses << " - mouse " << static_cast<int>(x)
              << ", " << static_cast<int>(y);
        window.setTitle(title.str());
    }
}

}  // namespace

int main()
{
    auto& ctx = WindowContext::get();

    auto left = WindowBuilder{}
                    .title("Left Window")
                    .size(560, 360)
                    .noGraphicsApi()
                    .resizable()
                    .build();
    auto right = WindowBuilder{}
                     .title("Right Window")
                     .size(560, 360)
                     .noGraphicsApi()
                     .resizable()
                     .build();

    left.setPosition(120, 160);
    right.setPosition(720, 160);

    int leftPresses = 0;
    int rightPresses = 0;
    EventDispatcher leftDispatcher = makeWindowDispatcher(left, "Left", leftPresses);
    EventDispatcher rightDispatcher = makeWindowDispatcher(right, "Right", rightPresses);
    FrameLimiter frameLimiter(120.0);

    std::cout << "Controls:\n"
              << "  Space: count a press in the focused window\n"
              << "  Esc: close the focused window\n";

    while (!left.shouldClose() || !right.shouldClose()) {
        ctx.pollEvents();

        leftDispatcher.dispatch(left.events());
        rightDispatcher.dispatch(right.events());
        updateWindowTitle(left, "Left", leftPresses);
        updateWindowTitle(right, "Right", rightPresses);

        frameLimiter.wait();
    }
}
