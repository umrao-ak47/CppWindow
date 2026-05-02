#include <cppwindow/cppwindow.hpp>

#include <cassert>
#include <thread>

int main()
{
    cwin::Clock clock;
    assert(clock.elapsedSeconds() >= 0.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    assert(clock.elapsedSeconds() > 0.0);
    assert(clock.restartSeconds() > 0.0);
    assert(clock.elapsedSeconds() >= 0.0);

    cwin::FrameTimer timer;
    const cwin::FrameTime first = timer.tick();
    assert(first.frameIndex == 0);
    assert(first.deltaSeconds == 0.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    const cwin::FrameTime second = timer.tick();
    assert(second.frameIndex == 1);
    assert(second.deltaSeconds > 0.0);
    assert(second.totalSeconds >= first.totalSeconds);

    cwin::FixedStepAccumulator accumulator(0.1);
    accumulator.add(0.05);
    assert(!accumulator.consumeStep());
    assert(accumulator.alpha() > 0.49);
    assert(accumulator.alpha() < 0.51);
    accumulator.add(0.16);
    assert(accumulator.consumeStep());
    assert(accumulator.consumeStep());
    assert(!accumulator.consumeStep());
    assert(accumulator.accumulatedSeconds() > 0.0);
    accumulator.reset();
    assert(accumulator.accumulatedSeconds() == 0.0);

    cwin::FixedStepAccumulator fallback(-1.0);
    assert(fallback.fixedDeltaSeconds() > 0.0);

    cwin::Event drop = cwin::Event::FilesDropped{
        .paths = { "/tmp/a.txt", "/tmp/b.txt" },
        .posX = 12.0,
        .posY = 24.0,
    };
    assert(drop.is<cwin::Event::FilesDropped>());
    const auto* files = drop.getIf<cwin::Event::FilesDropped>();
    assert(files);
    assert(files->paths.size() == 2);
    assert(files->posX == 12.0);
    assert(files->posY == 24.0);
}
