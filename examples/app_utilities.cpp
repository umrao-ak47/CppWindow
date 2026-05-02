#include <cppwindow/cppwindow.hpp>

#include <iomanip>
#include <iostream>
#include <sstream>

using namespace cwin;

int main()
{
    auto& ctx = WindowContext::Get();

    auto window = WindowBuilder{}.title("App Utilities").size(840, 420).noAPI().resizable().build();

    std::cout << "Keys: C copy text, V print clipboard, Esc close. Drop files onto the window.\n";

    FrameTimer frameTimer;
    FixedStepAccumulator fixedStep(1.0 / 60.0);
    FpsCounter fpsCounter(0.5);
    FrameLimiter frameLimiter(60.0);
    uint64_t fixedSteps = 0;
    double displayedFps = 0.0;

    while (!window.shouldClose()) {
        const FrameTime frame = frameTimer.tick();
        if (fpsCounter.update(frame)) {
            displayedFps = fpsCounter.framesPerSecond();
        }
        fixedStep.add(frame.deltaSeconds);
        while (fixedStep.consumeStep()) {
            ++fixedSteps;
        }

        ctx.pollEvents();

        for (const auto& event : window.events()) {
            if (event.is<Event::Closed>()) {
                window.requestClose();
            }

            if (const auto* key = event.getIf<Event::KeyPressed>()) {
                if (key->key == Key::Escape) {
                    window.requestClose();
                } else if (key->key == Key::C) {
                    std::ostringstream text;
                    text << "Copied from CppWindow frame " << frame.frameIndex;
                    ctx.setClipboardText(text.str());
                    std::cout << "clipboard set: " << text.str() << "\n";
                } else if (key->key == Key::V) {
                    std::cout << "clipboard: " << ctx.getClipboardText() << "\n";
                }
            }

            if (const auto* drop = event.getIf<Event::FilesDropped>()) {
                std::cout << "files dropped at " << drop->posX << ", " << drop->posY << "\n";
                for (const auto& path : drop->paths) {
                    std::cout << "  " << path << "\n";
                }
            }
        }

        std::ostringstream title;
        title << "App Utilities - frame " << frame.frameIndex << " dt " << std::fixed
              << std::setprecision(3) << frame.deltaSeconds << " fps " << std::setprecision(1)
              << displayedFps << " fixed " << fixedSteps << " alpha " << std::setprecision(2)
              << fixedStep.alpha();
        window.setTitle(title.str());

        frameLimiter.wait();
    }
}
