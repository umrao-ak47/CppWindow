#include <cppwindow/cppwindow.hpp>

#include <iostream>

using namespace cwin;

namespace {

const char* systemName(NativeHandles::System system)
{
    switch (system) {
        case NativeHandles::System::Win32:
            return "Win32";
        case NativeHandles::System::Cocoa:
            return "Cocoa";
        case NativeHandles::System::X11:
            return "X11";
        case NativeHandles::System::Wayland:
            return "Wayland";
        case NativeHandles::System::Unknown:
            return "Unknown";
    }

    return "Unknown";
}

void printNativeHandles(const Window& window)
{
    const NativeHandles handles = window.nativeHandles();

    std::cout << "native system: " << systemName(handles.system) << "\n";
    std::cout << "window handle: " << handles.window << "\n";
    std::cout << "display handle: " << handles.display << "\n";
}

void printVulkanInfo(const WindowContext& ctx)
{
    std::cout << "vulkan supported: " << ctx.isVulkanSupported() << "\n";

    const auto extensions = ctx.requiredVulkanInstanceExtensions();
    std::cout << "required GLFW Vulkan extensions: " << extensions.size() << "\n";
    for (const auto& extension : extensions) {
        std::cout << "  " << extension << "\n";
    }
}

}  // namespace

int main()
{
    auto& ctx = WindowContext::get();
    auto window =
        WindowBuilder{}.title("Native Handles").size(760, 360).noGraphicsApi().resizable().build();

    std::cout << "Controls:\n"
              << "  H: print native handles\n"
              << "  V: print Vulkan support\n"
              << "  Esc: close\n";
    printNativeHandles(window);
    printVulkanInfo(ctx);

    FrameLimiter frameLimiter(60.0);
    EventDispatcher dispatcher;
    dispatcher
        .on<Event::Closed>([&] {
            window.requestClose();
        })
        .on<Event::KeyPressed>([&](const Event::KeyPressed& key) {
            if (key.key == Key::Escape) {
                window.requestClose();
            } else if (key.key == Key::H) {
                printNativeHandles(window);
            } else if (key.key == Key::V) {
                printVulkanInfo(ctx);
            }
        });

    while (!window.shouldClose()) {
        ctx.pollEvents();
        dispatcher.dispatch(window.events());

        frameLimiter.wait();
    }
}
