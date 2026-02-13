#include <cppwindow/cppwindow.hpp>

#include <glad/glad.h>
#include <iostream>
#include <stdexcept>

using namespace cppwindow;

int main()
{
    auto& ctx = WindowContext::Get();

    auto window = WindowBuilder{}
                      .title("OpenGL Example")
                      .size(1280, 720)
                      .openGL({ 4, 5, true })
                      .resizable()
                      .build();
    window.makeContextCurrent();

    // Load OpenGL
    if (!gladLoadGLLoader((GLADloadproc)ctx.getProcLoader())) {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    int major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    std::cout << "OpenGL Version : " << major << "." << minor << "\n";

    while (!window.shouldClose()) {
        ctx.pollEvents();

        for (auto& e : window.events()) {
            if (e.is<Event::Closed>()) {
                window.requestClose();
            }
        }

        glClearColor(0.5f, 0.1f, 0.9f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        window.swapBuffers();
    }
}