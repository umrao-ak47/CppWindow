#include <cppwindow/cppwindow.hpp>

#include <glad/glad.h>
#include <iostream>
#include <stdexcept>

using namespace cwin;

namespace {

const char* cursorModeName(CursorMode mode)
{
    switch (mode) {
        case CursorMode::Normal:
            return "normal";
        case CursorMode::Hidden:
            return "hidden";
        case CursorMode::Captured:
            return "captured";
    }

    return "unknown";
}

}  // namespace

int main()
{
    auto& ctx = WindowContext::Get();

    auto window = WindowBuilder{}
                      .title("Window Controls")
                      .size(960, 540)
                      .openGL({ 4, 1, true })
                      .resizable()
                      .build();
    window.makeContextCurrent();

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(ctx.getProcLoader()))) {
        throw std::runtime_error("Failed to initialize GLAD");
    }
    const bool useVSync = true;
    window.setVSync(useVSync);
    FrameLimiter frameLimiter(120.0);
    frameLimiter.setVSyncEnabled(useVSync);

    std::cout << "Monitors:\n";
    for (const auto& monitor : ctx.getMonitors()) {
        std::cout << "  [" << monitor.id << "] " << monitor.name << " "
                  << monitor.currentVideoMode.width << "x" << monitor.currentVideoMode.height
                  << " scale " << monitor.contentScaleX << "x" << monitor.contentScaleY;
        if (monitor.primary) {
            std::cout << " primary";
        }
        std::cout << "\n";
    }

    std::cout << "\nKeys: Esc close, R resizable, D decorated, T floating, C cursor, "
                 "O/P opacity, L/K size limits, A/S aspect ratio\n";

    bool resizable = true;
    bool decorated = true;
    bool floating = false;
    float opacity = 1.0f;
    CursorMode cursorMode = CursorMode::Normal;

    while (!window.shouldClose()) {
        ctx.pollEvents();

        for (const auto& event : window.events()) {
            if (event.is<Event::Closed>()) {
                window.requestClose();
            }

            if (const auto* key = event.getIf<Event::KeyPressed>()) {
                if (key->key == Key::Escape) {
                    window.requestClose();
                } else if (key->key == Key::R) {
                    resizable = !resizable;
                    window.setResizable(resizable);
                    std::cout << "resizable: " << resizable << "\n";
                } else if (key->key == Key::D) {
                    decorated = !decorated;
                    window.setDecorated(decorated);
                    std::cout << "decorated: " << decorated << "\n";
                } else if (key->key == Key::T) {
                    floating = !floating;
                    window.setFloating(floating);
                    std::cout << "floating: " << floating << "\n";
                } else if (key->key == Key::C) {
                    cursorMode = cursorMode == CursorMode::Normal   ? CursorMode::Hidden
                                 : cursorMode == CursorMode::Hidden ? CursorMode::Captured
                                                                    : CursorMode::Normal;
                    window.setCursorMode(cursorMode);
                    std::cout << "cursor: " << cursorModeName(cursorMode) << "\n";
                } else if (key->key == Key::O) {
                    opacity -= 0.1f;
                    window.setOpacity(opacity);
                    opacity = window.getOpacity();
                    std::cout << "opacity: " << opacity << "\n";
                } else if (key->key == Key::P) {
                    opacity += 0.1f;
                    window.setOpacity(opacity);
                    opacity = window.getOpacity();
                    std::cout << "opacity: " << opacity << "\n";
                } else if (key->key == Key::L) {
                    window.setSizeLimits(
                        SizeLimits{
                            .minWidth = 640,
                            .minHeight = 360,
                            .maxWidth = 1600,
                            .maxHeight = 900,
                        });
                    std::cout << "size limits set\n";
                } else if (key->key == Key::K) {
                    window.clearSizeLimits();
                    std::cout << "size limits cleared\n";
                } else if (key->key == Key::A) {
                    window.setAspectRatio({ 16, 9 });
                    std::cout << "aspect ratio: 16:9\n";
                } else if (key->key == Key::S) {
                    window.clearAspectRatio();
                    std::cout << "aspect ratio cleared\n";
                }
            }

            if (const auto* moved = event.getIf<Event::Moved>()) {
                std::cout << "moved: " << moved->x << ", " << moved->y << "\n";
            }
            if (const auto* scale = event.getIf<Event::ContentScaleChanged>()) {
                std::cout << "content scale: " << scale->xScale << "x" << scale->yScale << "\n";
            }
        }

        auto [fbWidth, fbHeight] = window.getFrameBufferSize();
        glViewport(0, 0, static_cast<GLsizei>(fbWidth), static_cast<GLsizei>(fbHeight));

        const float decorationTint = decorated ? 0.10f : 0.42f;
        const float floatingTint = floating ? 0.18f : 0.08f;
        glClearColor(decorated ? 0.04f : 0.12f, decorationTint, floatingTint, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        window.swapBuffers();
        frameLimiter.wait();
    }
}
