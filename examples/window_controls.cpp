#include <cppwindow/cppwindow.hpp>

#include <cstddef>
#include <cstdint>
#include <glad/glad.h>
#include <iostream>
#include <span>
#include <stdexcept>
#include <vector>

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

std::vector<uint8_t> makeIconPixels(uint32_t width, uint32_t height)
{
    std::vector<uint8_t> pixels(static_cast<size_t>(width) * height * 4);
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            const bool border = x < 2 || y < 2 || x + 2 >= width || y + 2 >= height;
            const bool diagonal = x == y || x + y + 1 == width;
            const size_t offset = (static_cast<size_t>(y) * width + x) * 4;
            pixels[offset + 0] = border ? 242 : diagonal ? 255 : 26;
            pixels[offset + 1] = border ? 244 : diagonal ? 184 : 115;
            pixels[offset + 2] = border ? 247 : diagonal ? 75 : 232;
            pixels[offset + 3] = 255;
        }
    }
    return pixels;
}

std::vector<uint8_t> makeCursorPixels(uint32_t width, uint32_t height)
{
    std::vector<uint8_t> pixels(static_cast<size_t>(width) * height * 4, 0);
    const int centerX = static_cast<int>(width / 2);
    const int centerY = static_cast<int>(height / 2);

    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            const int px = static_cast<int>(x);
            const int py = static_cast<int>(y);
            const bool outline =
                px == centerX - 1 || px == centerX + 1 || py == centerY - 1 || py == centerY + 1;
            const bool line = px == centerX || py == centerY;
            if (!outline && !line) {
                continue;
            }

            const size_t offset = (static_cast<size_t>(y) * width + x) * 4;
            pixels[offset + 0] = line ? 255 : 16;
            pixels[offset + 1] = line ? 255 : 24;
            pixels[offset + 2] = line ? 255 : 36;
            pixels[offset + 3] = line ? 255 : 220;
        }
    }
    return pixels;
}

cwin::ImageRgba imageFrom(const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height)
{
    return cwin::ImageRgba{
        .width = width,
        .height = height,
        .pixels = std::span<const uint8_t>{ pixels.data(), pixels.size() },
    };
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

    const auto iconPixels = makeIconPixels(32, 32);
    const auto cursorPixels = makeCursorPixels(24, 24);
    if (!window.setIcon(imageFrom(iconPixels, 32, 32))) {
        std::cout << "window icon not supported by this platform/backend\n";
    }

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

    std::cout << "\nControls:\n"
              << "  R: toggle resizable\n"
              << "  D: toggle decorated\n"
              << "  T: toggle floating\n"
              << "  C: cycle cursor mode\n"
              << "  H: set hand cursor\n"
              << "  J: set custom cursor\n"
              << "  N: restore default cursor\n"
              << "  I: set window icon\n"
              << "  M: restore default icon\n"
              << "  U: request attention\n"
              << "  O: decrease opacity\n"
              << "  P: increase opacity\n"
              << "  L: set size limits\n"
              << "  K: clear size limits\n"
              << "  A: set 16:9 aspect ratio\n"
              << "  S: clear aspect ratio\n"
              << "  Esc: close\n";

    bool resizable = true;
    bool decorated = true;
    bool floating = false;
    float opacity = 1.0f;
    CursorMode cursorMode = CursorMode::Normal;
    EventDispatcher dispatcher;
    dispatcher
        .on<Event::Closed>([&] {
            window.requestClose();
        })
        .on<Event::KeyPressed>([&](const Event::KeyPressed& key) {
            if (key.key == Key::Escape) {
                window.requestClose();
            } else if (key.key == Key::R) {
                resizable = !resizable;
                window.setResizable(resizable);
                std::cout << "resizable: " << resizable << "\n";
            } else if (key.key == Key::D) {
                decorated = !decorated;
                window.setDecorated(decorated);
                std::cout << "decorated: " << decorated << "\n";
            } else if (key.key == Key::T) {
                floating = !floating;
                window.setFloating(floating);
                std::cout << "floating: " << floating << "\n";
            } else if (key.key == Key::C) {
                cursorMode = cursorMode == CursorMode::Normal   ? CursorMode::Hidden
                             : cursorMode == CursorMode::Hidden ? CursorMode::Captured
                                                                : CursorMode::Normal;
                window.setCursorMode(cursorMode);
                std::cout << "cursor: " << cursorModeName(cursorMode) << "\n";
            } else if (key.key == Key::H) {
                cursorMode = CursorMode::Normal;
                window.setCursorMode(cursorMode);
                std::cout << "hand cursor: " << window.setCursorShape(CursorShape::Hand) << "\n";
            } else if (key.key == Key::J) {
                cursorMode = CursorMode::Normal;
                window.setCursorMode(cursorMode);
                std::cout << "custom cursor: "
                          << window.setCursorImage(imageFrom(cursorPixels, 24, 24), 12, 12) << "\n";
            } else if (key.key == Key::N) {
                window.clearCursor();
                std::cout << "cursor image cleared\n";
            } else if (key.key == Key::I) {
                std::cout << "icon set: " << window.setIcon(imageFrom(iconPixels, 32, 32)) << "\n";
            } else if (key.key == Key::M) {
                window.clearIcon();
                std::cout << "icon cleared\n";
            } else if (key.key == Key::U) {
                window.requestAttention();
                std::cout << "attention requested\n";
            } else if (key.key == Key::O) {
                opacity -= 0.1f;
                window.setOpacity(opacity);
                opacity = window.getOpacity();
                std::cout << "opacity: " << opacity << "\n";
            } else if (key.key == Key::P) {
                opacity += 0.1f;
                window.setOpacity(opacity);
                opacity = window.getOpacity();
                std::cout << "opacity: " << opacity << "\n";
            } else if (key.key == Key::L) {
                window.setSizeLimits(
                    SizeLimits{
                        .minWidth = 640,
                        .minHeight = 360,
                        .maxWidth = 1600,
                        .maxHeight = 900,
                    });
                std::cout << "size limits set\n";
            } else if (key.key == Key::K) {
                window.clearSizeLimits();
                std::cout << "size limits cleared\n";
            } else if (key.key == Key::A) {
                window.setAspectRatio({ 16, 9 });
                std::cout << "aspect ratio: 16:9\n";
            } else if (key.key == Key::S) {
                window.clearAspectRatio();
                std::cout << "aspect ratio cleared\n";
            }
        })
        .on<Event::Moved>([](const Event::Moved& moved) {
            std::cout << "moved: " << moved.x << ", " << moved.y << "\n";
        })
        .on<Event::ContentScaleChanged>([](const Event::ContentScaleChanged& scale) {
            std::cout << "content scale: " << scale.xScale << "x" << scale.yScale << "\n";
        });

    while (!window.shouldClose()) {
        ctx.pollEvents();
        dispatcher.dispatch(window.events());

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
