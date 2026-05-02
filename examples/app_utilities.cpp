#include <cppwindow/cppwindow.hpp>

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace cwin;

int main()
{
    auto& ctx = WindowContext::Get();

    auto window = WindowBuilder{}.title("App Utilities").size(840, 420).noAPI().resizable().build();

    std::cout << "Controls:\n"
              << "  C: copy text\n"
              << "  V: print clipboard\n"
              << "  Drop files: print dropped paths\n"
              << "  Esc: close\n";

    FrameTimer frameTimer;
    FixedStepAccumulator fixedStep(1.0 / 60.0);
    FpsCounter fpsCounter(0.5);
    FrameLimiter frameLimiter(60.0);
    uint64_t fixedSteps = 0;
    uint64_t currentFrameIndex = 0;
    double displayedFps = 0.0;
    EventDispatcher dispatcher;
    dispatcher
        .on<Event::Closed>([&] {
            window.requestClose();
        })
        .on<Event::KeyPressed>([&](const Event::KeyPressed& key) {
            if (key.key == Key::Escape) {
                window.requestClose();
            } else if (key.key == Key::C) {
                std::ostringstream text;
                text << "Copied from CppWindow frame " << currentFrameIndex;
                if (ctx.setClipboardText(text.str())) {
                    std::cout << "clipboard set: " << text.str() << "\n";
                } else {
                    std::cout << "clipboard set failed\n";
                }
            } else if (key.key == Key::V) {
                if (auto text = ctx.tryGetClipboardText()) {
                    std::cout << "clipboard: " << *text << "\n";
                } else {
                    std::cout << "clipboard unavailable\n";
                }
            }
        })
        .on<Event::FilesDropped>([](const Event::FilesDropped& drop) {
            std::cout << "files dropped at " << drop.posX << ", " << drop.posY << "\n";
            for (const auto& path : drop.paths) {
                std::cout << "  " << path << "\n";
            }
        });

    while (!window.shouldClose()) {
        const FrameTime frame = frameTimer.tick();
        currentFrameIndex = frame.frameIndex;
        if (fpsCounter.update(frame)) {
            displayedFps = fpsCounter.framesPerSecond();
        }
        fixedStep.add(frame.deltaSeconds);
        while (fixedStep.consumeStep()) {
            ++fixedSteps;
        }

        ctx.pollEvents();
        dispatcher.dispatch(window.events());

        std::ostringstream title;
        title << "App Utilities - frame " << frame.frameIndex << " dt " << std::fixed
              << std::setprecision(3) << frame.deltaSeconds << " fps " << std::setprecision(1)
              << displayedFps << " fixed " << fixedSteps << " alpha " << std::setprecision(2)
              << fixedStep.alpha();
        window.setTitle(title.str());

        frameLimiter.wait();
    }
}
