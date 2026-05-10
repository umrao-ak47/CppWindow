/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

/// @file timing.hpp
/// @brief Timing helpers for frame loops and pacing.

#ifndef CPPWINDOW_HEADER_TIMING_HPP
#define CPPWINDOW_HEADER_TIMING_HPP

#include <chrono>
#include <cstdint>

namespace cwin {

//----------------------------------------------------------------------------
//  Timing
//----------------------------------------------------------------------------
/// Monotonic elapsed-time clock.
class Clock final
{
public:
    /// Underlying steady clock type.
    using SteadyClock = std::chrono::steady_clock;
    /// Underlying duration type.
    using Duration = SteadyClock::duration;
    /// Underlying time point type.
    using TimePoint = SteadyClock::time_point;

    /// Starts a clock at the current time.
    Clock() noexcept;

    /// Resets the start time to now.
    void reset() noexcept;
    /// Returns elapsed duration since construction or last reset.
    [[nodiscard]] Duration elapsed() const noexcept;
    /// Returns elapsed seconds since construction or last reset.
    [[nodiscard]] double elapsedSeconds() const noexcept;
    /// Returns elapsed seconds and resets the start time to now.
    [[nodiscard]] double restartSeconds() noexcept;

private:
    TimePoint start_;
};

/// Per-frame timing result.
struct FrameTime
{
    /// Seconds since the previous frame tick.
    double deltaSeconds = 0.0;
    /// Seconds since the frame timer was created or reset.
    double totalSeconds = 0.0;
    /// Zero-based frame index.
    uint64_t frameIndex = 0;
};

/// Frame timer that leaves the application loop under caller control.
class FrameTimer final
{
public:
    /// Creates a frame timer starting at now.
    FrameTimer() noexcept;

    /// Resets elapsed time and frame index.
    void reset() noexcept;
    /// Advances and returns the next frame timing sample.
    [[nodiscard]] FrameTime tick() noexcept;

private:
    Clock clock_;
    double lastSeconds_ = 0.0;
    uint64_t frameIndex_ = 0;
};

/// Fixed-step simulation accumulator.
class FixedStepAccumulator final
{
public:
    /// Creates an accumulator with the requested fixed step duration.
    explicit FixedStepAccumulator(double fixedDeltaSeconds = 1.0 / 60.0) noexcept;

    /// Clears accumulated time.
    void reset() noexcept;
    /// Adds positive elapsed time to the accumulator.
    void add(double deltaSeconds) noexcept;
    /// Consumes one fixed step if enough time is accumulated.
    [[nodiscard]] bool consumeStep() noexcept;
    /// Returns interpolation alpha in fixed-step units.
    [[nodiscard]] double alpha() const noexcept;
    /// Returns accumulated seconds not yet consumed.
    [[nodiscard]] double accumulatedSeconds() const noexcept;
    /// Returns fixed step duration in seconds.
    [[nodiscard]] double fixedDeltaSeconds() const noexcept;

private:
    double fixedDeltaSeconds_ = 1.0 / 60.0;
    double accumulatedSeconds_ = 0.0;
};

/// Periodically samples frames per second from caller-provided frame deltas.
class FpsCounter final
{
public:
    /// Creates a counter that refreshes reported FPS after the update interval.
    explicit FpsCounter(double updateIntervalSeconds = 0.5) noexcept;

    /// Clears all accumulated and reported state.
    void reset() noexcept;
    /// Adds one frame delta and returns true when a new FPS sample is available.
    [[nodiscard]] bool update(double deltaSeconds) noexcept;
    /// Adds one frame timing sample and returns true when a new FPS sample is available.
    [[nodiscard]] bool update(const FrameTime& frameTime) noexcept;
    /// Returns the last sampled frames per second.
    [[nodiscard]] double framesPerSecond() const noexcept;
    /// Returns the last sampled average frame duration.
    [[nodiscard]] double frameSeconds() const noexcept;
    /// Returns the configured sampling interval in seconds.
    [[nodiscard]] double updateIntervalSeconds() const noexcept;
    /// Returns total frames passed to `update`.
    [[nodiscard]] uint64_t totalFrames() const noexcept;

private:
    double updateIntervalSeconds_ = 0.5;
    double accumulatedSeconds_ = 0.0;
    uint64_t accumulatedFrames_ = 0;
    uint64_t totalFrames_ = 0;
    double framesPerSecond_ = 0.0;
    double frameSeconds_ = 0.0;
};

/// Optional frame pacing helper that never owns the application loop.
class FrameLimiter final
{
public:
    /// Creates a limiter. A target FPS of zero disables sleep pacing.
    explicit FrameLimiter(double targetFramesPerSecond = 0.0) noexcept;

    /// Resets scheduling state.
    void reset() noexcept;
    /// Sets target FPS. Values less than or equal to zero disable sleep pacing.
    void setTargetFramesPerSecond(double framesPerSecond) noexcept;
    /// Disables sleep pacing.
    void clearTargetFramesPerSecond() noexcept;
    /// Returns target FPS, or zero when sleep pacing is disabled.
    [[nodiscard]] double targetFramesPerSecond() const noexcept;
    /// Returns target frame duration, or zero when sleep pacing is disabled.
    [[nodiscard]] double targetFrameSeconds() const noexcept;
    /// Records whether presentation is already paced by VSync.
    void setVSyncEnabled(bool enabled) noexcept;
    /// Returns whether VSync pacing is enabled.
    [[nodiscard]] bool isVSyncEnabled() const noexcept;
    /// Sleeps until the next target frame time unless disabled or VSync-paced.
    void wait() noexcept;

private:
    double targetFrameSeconds_ = 0.0;
    bool vSyncEnabled_ = false;
    bool started_ = false;
    Clock::TimePoint nextFrameTime_{};
};

}  // namespace cwin

#endif
