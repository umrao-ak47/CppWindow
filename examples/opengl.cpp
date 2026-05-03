#include <cppwindow/cppwindow.hpp>

#include <glad/glad.h>
#include <iostream>
#include <stdexcept>

using namespace cwin;

int main()
{
    auto& ctx = WindowContext::get();

    auto window = WindowBuilder{}
                      .title("OpenGL Example")
                      .size(1280, 720)
                      .openGL({ 4, 1, true })
                      .resizable()
                      .build();
    window.makeContextCurrent();

    // Load OpenGL
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(ctx.getProcLoader()))) {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    int major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    std::cout << "OpenGL Version : " << major << "." << minor << "\n";

    const bool useVSync = true;
    window.setVSync(useVSync);
    FrameLimiter frameLimiter(60.0);
    frameLimiter.setVSyncEnabled(useVSync);
    EventDispatcher dispatcher;
    dispatcher.on<Event::Closed>([&] {
        window.requestClose();
    });

    while (!window.shouldClose()) {
        ctx.pollEvents();
        dispatcher.dispatch(window.events());

        auto [fbWidth, fbHeight] = window.getFramebufferSize();
        glViewport(0, 0, static_cast<GLsizei>(fbWidth), static_cast<GLsizei>(fbHeight));

        glClearColor(0.5f, 0.1f, 0.9f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        window.swapBuffers();
        frameLimiter.wait();
    }
}
