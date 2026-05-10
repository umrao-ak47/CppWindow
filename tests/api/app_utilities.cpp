#include <cppwindow/cppwindow.hpp>

#include <cassert>
#include <cmath>
#include <limits>
#include <thread>

namespace {

[[nodiscard]] bool closeTo(double actual, double expected)
{
    return std::abs(actual - expected) < 0.000001;
}

}  // namespace

int main()
{
    cwin::DpiScale dpi{
        .x = 2.0f,
        .y = 1.5f,
    };
    auto [fbX, fbY] = dpi.windowToFramebuffer(10.0, 20.0);
    assert(closeTo(fbX, 20.0));
    assert(closeTo(fbY, 30.0));

    auto [windowX, windowY] = dpi.framebufferToWindow(20.0, 30.0);
    assert(closeTo(windowX, 10.0));
    assert(closeTo(windowY, 20.0));

    auto [fbWidth, fbHeight] = dpi.windowSizeToFramebuffer(320, 200);
    assert(fbWidth == 640);
    assert(fbHeight == 300);

    auto [windowWidth, windowHeight] = dpi.framebufferSizeToWindow(640.0, 300.0);
    assert(closeTo(windowWidth, 320.0));
    assert(closeTo(windowHeight, 200.0));

    cwin::DpiScale invalidDpi{
        .x = std::numeric_limits<float>::infinity(),
        .y = -1.0f,
    };
    auto [sameX, sameY] = invalidDpi.windowToFramebuffer(12.5, 7.5);
    assert(closeTo(sameX, 12.5));
    assert(closeTo(sameY, 7.5));

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

    cwin::FpsCounter fps(0.5);
    assert(!fps.update(0.1));
    assert(fps.totalFrames() == 1);
    assert(fps.framesPerSecond() == 0.0);
    assert(fps.update(0.4));
    assert(fps.totalFrames() == 2);
    assert(closeTo(fps.framesPerSecond(), 4.0));
    assert(closeTo(fps.frameSeconds(), 0.25));
    cwin::FrameTime invalidFrame{
        .deltaSeconds = -1.0,
        .totalSeconds = 0.0,
        .frameIndex = 0,
    };
    assert(!fps.update(invalidFrame));
    assert(fps.totalFrames() == 3);
    fps.reset();
    assert(fps.totalFrames() == 0);
    assert(fps.framesPerSecond() == 0.0);
    assert(fps.frameSeconds() == 0.0);

    cwin::FpsCounter fallbackFps(std::numeric_limits<double>::infinity());
    assert(closeTo(fallbackFps.updateIntervalSeconds(), 0.5));

    cwin::FrameLimiter limiter(120.0);
    assert(closeTo(limiter.targetFramesPerSecond(), 120.0));
    assert(closeTo(limiter.targetFrameSeconds(), 1.0 / 120.0));
    limiter.setVSyncEnabled(true);
    assert(limiter.isVSyncEnabled());
    limiter.wait();
    limiter.setTargetFramesPerSecond(60.0);
    assert(closeTo(limiter.targetFramesPerSecond(), 60.0));
    assert(limiter.isVSyncEnabled());
    limiter.setVSyncEnabled(false);
    assert(!limiter.isVSyncEnabled());
    limiter.clearTargetFramesPerSecond();
    assert(limiter.targetFramesPerSecond() == 0.0);
    assert(limiter.targetFrameSeconds() == 0.0);
    limiter.wait();
    limiter.setTargetFramesPerSecond(-1.0);
    assert(limiter.targetFramesPerSecond() == 0.0);
    limiter.setTargetFramesPerSecond(std::numeric_limits<double>::infinity());
    assert(limiter.targetFramesPerSecond() == 0.0);

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
