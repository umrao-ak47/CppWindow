/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#include <cppwindow/cppwindow.hpp>

#include <chrono>
#include <cmath>
#include <thread>

namespace cwin {

//----------------------------------------------------------------------------
//  Timing Implementation
//----------------------------------------------------------------------------
Clock::Clock() noexcept
    : start_(SteadyClock::now())
{
}

void Clock::reset() noexcept
{
    start_ = SteadyClock::now();
}

Clock::Duration Clock::elapsed() const noexcept
{
    return SteadyClock::now() - start_;
}

double Clock::elapsedSeconds() const noexcept
{
    return std::chrono::duration<double>(elapsed()).count();
}

double Clock::restartSeconds() noexcept
{
    const auto now = SteadyClock::now();
    const double seconds = std::chrono::duration<double>(now - start_).count();
    start_ = now;
    return seconds;
}

FrameTimer::FrameTimer() noexcept = default;

void FrameTimer::reset() noexcept
{
    clock_.reset();
    lastSeconds_ = 0.0;
    frameIndex_ = 0;
}

FrameTime FrameTimer::tick() noexcept
{
    const double totalSeconds = clock_.elapsedSeconds();
    const double deltaSeconds = frameIndex_ == 0 ? 0.0 : totalSeconds - lastSeconds_;
    const uint64_t frameIndex = frameIndex_;

    lastSeconds_ = totalSeconds;
    ++frameIndex_;

    return FrameTime{
        .deltaSeconds = deltaSeconds,
        .totalSeconds = totalSeconds,
        .frameIndex = frameIndex,
    };
}

FixedStepAccumulator::FixedStepAccumulator(double fixedDeltaSeconds) noexcept
    : fixedDeltaSeconds_(fixedDeltaSeconds > 0.0 ? fixedDeltaSeconds : 1.0 / 60.0)
{
}

void FixedStepAccumulator::reset() noexcept
{
    accumulatedSeconds_ = 0.0;
}

void FixedStepAccumulator::add(double deltaSeconds) noexcept
{
    if (deltaSeconds > 0.0) {
        accumulatedSeconds_ += deltaSeconds;
    }
}

bool FixedStepAccumulator::consumeStep() noexcept
{
    if (accumulatedSeconds_ < fixedDeltaSeconds_) {
        return false;
    }

    accumulatedSeconds_ -= fixedDeltaSeconds_;
    return true;
}

double FixedStepAccumulator::alpha() const noexcept
{
    return fixedDeltaSeconds_ > 0.0 ? accumulatedSeconds_ / fixedDeltaSeconds_ : 0.0;
}

double FixedStepAccumulator::accumulatedSeconds() const noexcept
{
    return accumulatedSeconds_;
}

double FixedStepAccumulator::fixedDeltaSeconds() const noexcept
{
    return fixedDeltaSeconds_;
}

FpsCounter::FpsCounter(double updateIntervalSeconds) noexcept
    : updateIntervalSeconds_(
          updateIntervalSeconds > 0.0 && std::isfinite(updateIntervalSeconds)
              ? updateIntervalSeconds
              : 0.5)
{
}

void FpsCounter::reset() noexcept
{
    accumulatedSeconds_ = 0.0;
    accumulatedFrames_ = 0;
    totalFrames_ = 0;
    framesPerSecond_ = 0.0;
    frameSeconds_ = 0.0;
}

bool FpsCounter::update(double deltaSeconds) noexcept
{
    ++totalFrames_;
    const double elapsedSeconds =
        deltaSeconds > 0.0 && std::isfinite(deltaSeconds) ? deltaSeconds : 0.0;
    if (elapsedSeconds <= 0.0) {
        return false;
    }

    ++accumulatedFrames_;
    accumulatedSeconds_ += elapsedSeconds;

    if (accumulatedSeconds_ < updateIntervalSeconds_ || accumulatedFrames_ == 0) {
        return false;
    }

    framesPerSecond_ = static_cast<double>(accumulatedFrames_) / accumulatedSeconds_;
    frameSeconds_ = accumulatedSeconds_ / static_cast<double>(accumulatedFrames_);
    accumulatedSeconds_ = 0.0;
    accumulatedFrames_ = 0;
    return true;
}

bool FpsCounter::update(const FrameTime& frameTime) noexcept
{
    return update(frameTime.deltaSeconds);
}

double FpsCounter::framesPerSecond() const noexcept
{
    return framesPerSecond_;
}

double FpsCounter::frameSeconds() const noexcept
{
    return frameSeconds_;
}

double FpsCounter::updateIntervalSeconds() const noexcept
{
    return updateIntervalSeconds_;
}

uint64_t FpsCounter::totalFrames() const noexcept
{
    return totalFrames_;
}

FrameLimiter::FrameLimiter(double targetFramesPerSecond) noexcept
{
    setTargetFramesPerSecond(targetFramesPerSecond);
}

void FrameLimiter::reset() noexcept
{
    started_ = false;
    nextFrameTime_ = Clock::TimePoint{};
}

void FrameLimiter::setTargetFramesPerSecond(double framesPerSecond) noexcept
{
    targetFrameSeconds_ =
        framesPerSecond > 0.0 && std::isfinite(framesPerSecond) ? 1.0 / framesPerSecond : 0.0;
    reset();
}

void FrameLimiter::clearTargetFramesPerSecond() noexcept
{
    targetFrameSeconds_ = 0.0;
    reset();
}

double FrameLimiter::targetFramesPerSecond() const noexcept
{
    return targetFrameSeconds_ > 0.0 ? 1.0 / targetFrameSeconds_ : 0.0;
}

double FrameLimiter::targetFrameSeconds() const noexcept
{
    return targetFrameSeconds_;
}

void FrameLimiter::setVSyncEnabled(bool enabled) noexcept
{
    vSyncEnabled_ = enabled;
    reset();
}

bool FrameLimiter::isVSyncEnabled() const noexcept
{
    return vSyncEnabled_;
}

void FrameLimiter::wait() noexcept
{
    if (vSyncEnabled_ || targetFrameSeconds_ <= 0.0) {
        reset();
        return;
    }

    const auto targetDuration = std::chrono::duration_cast<Clock::Duration>(
        std::chrono::duration<double>(targetFrameSeconds_));
    const Clock::TimePoint now = Clock::SteadyClock::now();

    if (!started_) {
        nextFrameTime_ = now + targetDuration;
        started_ = true;
    } else {
        nextFrameTime_ += targetDuration;
        if (now > nextFrameTime_ + targetDuration) {
            nextFrameTime_ = now + targetDuration;
        }
    }

    if (now < nextFrameTime_) {
        std::this_thread::sleep_until(nextFrameTime_);
    }
}

}  // namespace cwin
