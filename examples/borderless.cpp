#include <cppwindow/cppwindow.hpp>

#include <iostream>

using namespace cwin;

int main()
{
    auto& ctx = Context::get();

    auto window =
        WindowBuilder{}
            .title("Borderless Example")
            .size(1280, 720)
            .openGL({ 4, 1, true })
            .borderless()
            .build();
    window.makeContextCurrent();

    const bool useVSync = true;
    window.setVSync(useVSync);
    FrameLimiter frameLimiter(60.0);
    frameLimiter.setVSyncEnabled(useVSync);
    EventDispatcher dispatcher;
    dispatcher
        .on<Event::Closed>([&] {
            window.requestClose();
        })
        .on<Event::KeyPressed>([&](const Event::KeyPressed& key) {
            if (key.key == Key::Escape) {
                std::cout << "Close window\n";
                window.requestClose();
            }
        });

    while (!window.shouldClose()) {
        ctx.pollEvents();
        dispatcher.dispatch(window.events());

        window.swapBuffers();
        frameLimiter.wait();
    }
}
