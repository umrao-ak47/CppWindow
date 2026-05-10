#include <cppwindow/cppwindow.hpp>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

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
    auto& ctx = Context::get();
    auto window =
        WindowBuilder{}.title("Fixed Step Loop").size(860, 360).noGraphicsApi().resizable().build();

    std::cout << "Fixed update runs at 60Hz. Rendering/presentation remains caller-controlled.\n"
              << "Controls:\n"
              << "  R: reset simulation\n"
              << "  Esc: close\n";

    FrameTimer frameTimer;
    FixedStepAccumulator fixedStep(1.0 / 60.0);
    FpsCounter fpsCounter(0.5);
    FrameLimiter frameLimiter(60.0);
    Simulation simulation;
    double displayedFps = 0.0;
    EventDispatcher dispatcher;
    dispatcher
        .on<Event::Closed>([&] {
            window.requestClose();
        })
        .on<Event::KeyPressed>([&](const Event::KeyPressed& key) {
            if (key.key == Key::Escape) {
                window.requestClose();
            } else if (key.key == Key::R) {
                fixedStep.reset();
                simulation = {};
            }
        });

    while (!window.shouldClose()) {
        const FrameTime frame = frameTimer.tick();
        const double clampedDelta = std::min(frame.deltaSeconds, 0.25);
        if (fpsCounter.update(frame)) {
            displayedFps = fpsCounter.framesPerSecond();
        }

        ctx.pollEvents();
        dispatcher.dispatch(window.events());

        fixedStep.add(clampedDelta);
        while (fixedStep.consumeStep()) {
            simulate(simulation, fixedStep.fixedDeltaSeconds());
        }

        std::ostringstream title;
        title << "Fixed Step Loop - frame " << frame.frameIndex << " dt " << std::fixed
              << std::setprecision(3) << clampedDelta << " fps " << std::setprecision(1)
              << displayedFps << " steps " << simulation.steps << " alpha " << std::setprecision(2)
              << fixedStep.alpha() << " pos " << static_cast<int>(simulation.position);
        window.setTitle(title.str());

        frameLimiter.wait();
    }
}
