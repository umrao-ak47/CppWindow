#include <cppwindow/cppwindow.hpp>

#include <iostream>
#include <sstream>

using namespace cwin;

namespace {

void printVideoMode(const VideoMode& mode)
{
    std::cout << mode.width << "x" << mode.height << " @" << mode.refreshRate << "Hz"
              << " rgb(" << mode.redBits << ", " << mode.greenBits << ", " << mode.blueBits
              << ")";
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

        std::cout << " pos=" << monitor.x << ", " << monitor.y << " scale="
                  << monitor.contentScaleX << ", " << monitor.contentScaleY << " physical="
                  << monitor.physicalWidthMM << "x" << monitor.physicalHeightMM << "mm current=";
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

    std::cout << "Press M to print monitor info again. Escape closes.\n";
    printMonitors(ctx);

    while (!window.shouldClose()) {
        ctx.pollEvents();

        for (const auto& event : window.events()) {
            if (event.is<Event::Closed>()) {
                window.requestClose();
            }

            if (const auto* key = event.getIf<Event::KeyPressed>()) {
                if (key->key == Key::Escape) {
                    window.requestClose();
                } else if (key->key == Key::M) {
                    printMonitors(ctx);
                }
            }
        }

        auto [width, height] = window.getSize();
        auto [fbWidth, fbHeight] = window.getFrameBufferSize();
        auto [scaleX, scaleY] = window.getContentScale();

        std::ostringstream title;
        title << "Monitor Info - size " << width << "x" << height << " framebuffer " << fbWidth
              << "x" << fbHeight << " scale " << scaleX << ", " << scaleY;
        window.setTitle(title.str());
    }
}
