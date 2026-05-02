#include <cppwindow/cppwindow.hpp>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>

using namespace cwin;

namespace {

struct Simulation
{
    double position = 0.0;
    double velocity = 220.0;
    uint64_t steps = 0;
};

void simulate(Simulation& simulation, double dt)
{
    simulation.position += simulation.velocity * dt;

    if (simulation.position > 600.0) {
        simulation.position = 600.0;
        simulation.velocity = -simulation.velocity;
    } else if (simulation.position < 0.0) {
        simulation.position = 0.0;
        simulation.velocity = -simulation.velocity;
    }

    ++simulation.steps;
}

}  // namespace

int main()
{
    auto& ctx = WindowContext::Get();
    auto window =
        WindowBuilder{}.title("Fixed Step Loop").size(860, 360).noAPI().resizable().build();

    std::cout << "Fixed update runs at 60Hz. Rendering/presentation remains caller-controlled.\n";

    FrameTimer frameTimer;
    FixedStepAccumulator fixedStep(1.0 / 60.0);
    Simulation simulation;

    while (!window.shouldClose()) {
        const FrameTime frame = frameTimer.tick();
        const double clampedDelta = std::min(frame.deltaSeconds, 0.25);

        ctx.pollEvents();

        for (const auto& event : window.events()) {
            if (event.is<Event::Closed>()) {
                window.requestClose();
            }

            if (const auto* key = event.getIf<Event::KeyPressed>()) {
                if (key->key == Key::Escape) {
                    window.requestClose();
                } else if (key->key == Key::R) {
                    fixedStep.reset();
                    simulation = {};
                }
            }
        }

        fixedStep.add(clampedDelta);
        while (fixedStep.consumeStep()) {
            simulate(simulation, fixedStep.fixedDeltaSeconds());
        }

        std::ostringstream title;
        title << "Fixed Step Loop - frame " << frame.frameIndex << " dt " << clampedDelta
              << " steps " << simulation.steps << " alpha " << fixedStep.alpha() << " pos "
              << static_cast<int>(simulation.position);
        window.setTitle(title.str());

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
