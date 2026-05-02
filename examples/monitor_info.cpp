#include <cppwindow/cppwindow.hpp>

#include <iostream>
#include <sstream>

using namespace cwin;

namespace {

void printVideoMode(const VideoMode& mode)
{
    std::cout << mode.width << "x" << mode.height << " @" << mode.refreshRate << "Hz"
              << " rgb(" << mode.redBits << ", " << mode.greenBits << ", " << mode.blueBits << ")";
}

void printMonitors(const WindowContext& ctx)
{
    const auto monitors = ctx.getMonitors();
    std::cout << "Monitors: " << monitors.size() << "\n";

    for (const auto& monitor : monitors) {
        std::cout << "[" << monitor.id << "] " << monitor.name;
        if (monitor.primary) {
            std::cout << " primary";
        }

        std::cout << " pos=" << monitor.x << ", " << monitor.y << " scale=" << monitor.contentScaleX
                  << ", " << monitor.contentScaleY << " physical=" << monitor.physicalWidthMM << "x"
                  << monitor.physicalHeightMM << "mm current=";
        printVideoMode(monitor.currentVideoMode);
        std::cout << "\n";

        const auto modes = ctx.getVideoModes(monitor.id);
        const std::size_t count = modes.size() < 5 ? modes.size() : 5;
        for (std::size_t i = 0; i < count; ++i) {
            std::cout << "  mode " << i << ": ";
            printVideoMode(modes[i]);
            std::cout << "\n";
        }
        if (modes.size() > count) {
            std::cout << "  ... " << (modes.size() - count) << " more modes\n";
        }
    }
}

}  // namespace

int main()
{
    auto& ctx = WindowContext::Get();
    auto window = WindowBuilder{}.title("Monitor Info").size(820, 420).noAPI().resizable().build();

    std::cout << "Controls:\n"
              << "  M: print monitor info again\n"
              << "  Esc: close\n";
    printMonitors(ctx);

    FrameLimiter frameLimiter(60.0);
    EventDispatcher dispatcher;
    dispatcher
        .on<Event::Closed>([&] {
            window.requestClose();
        })
        .on<Event::KeyPressed>([&](const Event::KeyPressed& key) {
            if (key.key == Key::Escape) {
                window.requestClose();
            } else if (key.key == Key::M) {
                printMonitors(ctx);
            }
        });

    while (!window.shouldClose()) {
        ctx.pollEvents();
        dispatcher.dispatch(window.events());

        auto [width, height] = window.getSize();
        auto [fbWidth, fbHeight] = window.getFrameBufferSize();
        const DpiScale dpi = window.getDpiScale();
        auto [scaledWidth, scaledHeight] = dpi.windowSizeToFramebuffer(width, height);

        std::ostringstream title;
        title << "Monitor Info - size " << width << "x" << height << " framebuffer " << fbWidth
              << "x" << fbHeight << " scale " << dpi.x << ", " << dpi.y << " dpi-size "
              << scaledWidth << "x" << scaledHeight;
        window.setTitle(title.str());

        frameLimiter.wait();
    }
}
