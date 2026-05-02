#include <cppwindow/cppwindow.hpp>

#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>

using namespace cwin;

int main()
{
    auto& ctx = WindowContext::Get();

    auto window =
        WindowBuilder{}.title("App Utilities").size(840, 420).noAPI().resizable().build();

    std::cout << "Keys: C copy text, V print clipboard, Esc close. Drop files onto the window.\n";

    FrameTimer frameTimer;
    FixedStepAccumulator fixedStep(1.0 / 60.0);
    uint64_t fixedSteps = 0;

    while (!window.shouldClose()) {
        const FrameTime frame = frameTimer.tick();
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
        title << "App Utilities - frame " << frame.frameIndex << " dt " << frame.deltaSeconds
              << " fixed " << fixedSteps << " alpha " << fixedStep.alpha();
        window.setTitle(title.str());

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}
