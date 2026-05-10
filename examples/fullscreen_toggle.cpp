#include <cppwindow/cppwindow.hpp>

#include <glad/glad.h>
#include <iostream>
#include <stdexcept>

using namespace cwin;

namespace {

enum class DemoMode
{
    Windowed,
    Fullscreen,
    BorderlessFullscreen,
    ExclusiveFullscreen
};

}  // namespace

int main()
{
    auto& ctx = Context::get();

    auto window =
        WindowBuilder{}
            .title("Fullscreen Toggle")
            .size(960, 540)
            .openGL({ 4, 1, true })
            .resizable()
            .build();
    window.makeContextCurrent();

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(ctx.procLoader()))) {
        throw std::runtime_error("Failed to initialize GLAD");
    }
    const bool useVSync = true;
    window.setVSync(useVSync);
    FrameLimiter frameLimiter(120.0);
    frameLimiter.setVSyncEnabled(useVSync);

    std::cout << "Controls:\n"
              << "  F: fullscreen with title bar\n"
              << "  B: borderless fullscreen\n"
              << "  E: exclusive fullscreen\n"
              << "  W: windowed\n"
              << "  Esc: close\n";

    DemoMode mode = DemoMode::Windowed;
    EventDispatcher dispatcher;
    dispatcher
        .on<Event::Closed>([&] {
            window.requestClose();
        })
        .on<Event::KeyPressed>([&](const Event::KeyPressed& key) {
            if (key.key == Key::Escape) {
                window.requestClose();
            } else if (key.key == Key::F) {
                if (mode != DemoMode::Fullscreen) {
                    mode = DemoMode::Fullscreen;
                    window.setWindowMode(WindowMode::Fullscreen);
                    std::cout << "fullscreen with title bar\n";
                }
            } else if (key.key == Key::B) {
                if (mode != DemoMode::BorderlessFullscreen) {
                    mode = DemoMode::BorderlessFullscreen;
                    window.setWindowMode(WindowMode::BorderlessFullscreen);
                    std::cout << "borderless fullscreen\n";
                }
            } else if (key.key == Key::W) {
                window.setWindowMode(WindowMode::Windowed);
                mode = DemoMode::Windowed;
                std::cout << "windowed\n";
            } else if (key.key == Key::E) {
                if (mode != DemoMode::ExclusiveFullscreen) {
                    mode = DemoMode::ExclusiveFullscreen;
                    window.setWindowMode(WindowMode::ExclusiveFullscreen);
                    std::cout << "exclusive fullscreen\n";
                }
            }
        });

    while (!window.shouldClose()) {
        ctx.pollEvents();
        dispatcher.dispatch(window.events());

        auto [fbWidth, fbHeight] = window.framebufferSize();
        glViewport(0, 0, static_cast<GLsizei>(fbWidth), static_cast<GLsizei>(fbHeight));

        if (mode == DemoMode::BorderlessFullscreen) {
            glClearColor(0.08f, 0.18f, 0.32f, 1.0f);
        } else if (mode == DemoMode::ExclusiveFullscreen) {
            glClearColor(0.28f, 0.10f, 0.10f, 1.0f);
        } else if (mode == DemoMode::Fullscreen) {
            glClearColor(0.08f, 0.22f, 0.12f, 1.0f);
        } else {
            glClearColor(0.06f, 0.10f, 0.12f, 1.0f);
        }
        glClear(GL_COLOR_BUFFER_BIT);

        window.swapBuffers();
        frameLimiter.wait();
    }
}
