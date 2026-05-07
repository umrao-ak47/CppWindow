/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#include <cppwindow/cppwindow.hpp>

#include <algorithm>
#include <cmath>
#include <thread>

#include "backend/active_backend.hpp"

namespace cwin {

//----------------------------------------------------------------------------
//  Error Implementation
//----------------------------------------------------------------------------
Error::Error(ErrorCode code, std::string message)
    : std::runtime_error(std::move(message)),
      code_(code)
{
}

ErrorCode Error::code() const noexcept
{
    return code_;
}

//----------------------------------------------------------------------------
//  DPI Implementation
//----------------------------------------------------------------------------
namespace {

[[nodiscard]] double sanitizedScale(float scale) noexcept
{
    return scale > 0.0f && std::isfinite(scale) ? static_cast<double>(scale) : 1.0;
}

}  // namespace

std::pair<double, double> DpiScale::windowToFramebuffer(double xPos, double yPos) const noexcept
{
    return {
        xPos * sanitizedScale(x),
        yPos * sanitizedScale(y),
    };
}

std::pair<double, double> DpiScale::framebufferToWindow(double xPos, double yPos) const noexcept
{
    return {
        xPos / sanitizedScale(x),
        yPos / sanitizedScale(y),
    };
}

std::pair<int, int> DpiScale::windowSizeToFramebuffer(int width, int height) const noexcept
{
    return {
        static_cast<int>(std::lround(width * sanitizedScale(x))),
        static_cast<int>(std::lround(height * sanitizedScale(y))),
    };
}

std::pair<double, double> DpiScale::framebufferSizeToWindow(double width, double height)
    const noexcept
{
    return framebufferToWindow(width, height);
}

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

//----------------------------------------------------------------------------
//  Gamepad State Implementation
//----------------------------------------------------------------------------
namespace {

[[nodiscard]] bool isValidGamepadButton(GamepadButton button) noexcept
{
    const auto idx = static_cast<size_t>(button);
    return idx < GamepadButtonCount;
}

[[nodiscard]] bool isValidGamepadAxis(GamepadAxis axis) noexcept
{
    const auto idx = static_cast<size_t>(axis);
    return idx < GamepadAxisCount;
}

}  // namespace

bool GamepadState::isButtonDown(GamepadButton button) const noexcept
{
    if (!isValidGamepadButton(button)) {
        return false;
    }

    return buttons[static_cast<size_t>(button)];
}

float GamepadState::axis(GamepadAxis axis) const noexcept
{
    if (!isValidGamepadAxis(axis)) {
        return 0.0f;
    }

    return axes[static_cast<size_t>(axis)];
}

//----------------------------------------------------------------------------
//  Input State Implemenation
//----------------------------------------------------------------------------
InputState::InputState(const InputStateData* state)
    : state_(state)
{
}

bool InputState::isKeyDown(Key key) const
{
    return state_->isKeyDown(key);
}

bool InputState::isKeyPressed(Key key) const
{
    return state_->isKeyPressed(key);
}

bool InputState::isKeyReleased(Key key) const
{
    return state_->isKeyReleased(key);
}

bool InputState::isMouseButtonDown(MouseButton button) const
{
    return state_->isMouseButtonDown(button);
}

bool InputState::isMouseButtonPressed(MouseButton button) const
{
    return state_->isMouseButtonPressed(button);
}

bool InputState::isMouseButtonReleased(MouseButton button) const
{
    return state_->isMouseButtonReleased(button);
}

std::pair<double, double> InputState::mousePosition() const
{
    return state_->mousePosition();
}

std::pair<double, double> InputState::mouseDelta() const
{
    return state_->mouseDelta();
}

std::pair<double, double> InputState::scrollDelta() const
{
    return state_->scrollDelta();
}

bool InputState::isMouseInside() const
{
    return state_->mouseInside;
}

//----------------------------------------------------------------------------
//  Action Map Implementation
//----------------------------------------------------------------------------
namespace {

template <typename T>
void appendUnique(std::vector<T>& values, T value)
{
    for (T existing : values) {
        if (existing == value) {
            return;
        }
    }

    values.push_back(value);
}

float sanitizeDeadzone(float deadzone) noexcept
{
    if (!std::isfinite(deadzone) || deadzone < 0.0f) {
        return 0.0f;
    }
    return std::clamp(deadzone, 0.0f, 1.0f);
}

}  // namespace

ActionId ActionMap::defineAction(std::string action)
{
    return ensureEntry(std::move(action)).id;
}

ActionId ActionMap::findAction(std::string_view action) const noexcept
{
    const Entry* entry = findEntry(action);
    return entry ? entry->id : ActionId{};
}

bool ActionMap::hasAction(std::string_view action) const noexcept
{
    return findEntry(action) != nullptr;
}

bool ActionMap::hasAction(ActionId action) const noexcept
{
    return findEntry(action) != nullptr;
}

std::vector<ActionInfo> ActionMap::actions() const
{
    std::vector<ActionInfo> actions;
    actions.reserve(entries_.size());

    for (const Entry& entry : entries_) {
        actions.push_back(ActionInfo{
            .id = entry.id,
            .name = entry.action,
            .metadata = entry.metadata,
            .binding = entry.binding,
            .down = entry.down,
            .pressed = entry.down && !entry.previousDown,
            .released = !entry.down && entry.previousDown,
            .axisValue = entry.axisValue,
        });
    }

    return actions;
}

ActionMap& ActionMap::setMetadata(ActionId action, ActionMetadata metadata)
{
    if (Entry* entry = findEntry(action)) {
        entry->metadata = std::move(metadata);
    }
    return *this;
}

const ActionMetadata* ActionMap::metadata(ActionId action) const noexcept
{
    const Entry* entry = findEntry(action);
    return entry ? &entry->metadata : nullptr;
}

ActionMap& ActionMap::bindKey(ActionId action, Key key, Modifiers modifiers, bool exactModifiers)
{
    if (Entry* entry = findEntry(action)) {
        appendUnique(
            entry->binding.keys,
            KeyBinding{
                .key = key,
                .modifiers = modifiers,
                .exactModifiers = exactModifiers,
            });
    }
    return *this;
}

ActionMap& ActionMap::bindKeyCombo(ActionId action, Key key, std::vector<Key> requiredKeys)
{
    if (Entry* entry = findEntry(action)) {
        appendUnique(
            entry->binding.keys,
            KeyBinding{
                .key = key,
                .requiredKeys = std::move(requiredKeys),
            });
    }
    return *this;
}

ActionMap& ActionMap::bindMouseButton(ActionId action, MouseButton button)
{
    if (Entry* entry = findEntry(action)) {
        appendUnique(entry->binding.mouseButtons, button);
    }
    return *this;
}

ActionMap& ActionMap::bindGamepadButton(ActionId action, GamepadButton button)
{
    if (Entry* entry = findEntry(action)) {
        appendUnique(entry->binding.gamepadButtons, button);
    }
    return *this;
}

ActionMap& ActionMap::bindGamepadAxis(
    ActionId action,
    GamepadAxis axis,
    float deadzone,
    AxisDirection direction)
{
    if (Entry* entry = findEntry(action)) {
        appendUnique(
            entry->binding.gamepadAxes,
            GamepadAxisBinding{
                .axis = axis,
                .deadzone = sanitizeDeadzone(deadzone),
                .direction = direction,
            });
    }
    return *this;
}

ActionMap&
ActionMap::replaceKey(ActionId action, Key key, Modifiers modifiers, bool exactModifiers)
{
    if (Entry* entry = findEntry(action)) {
        entry->binding.keys.clear();
        appendUnique(
            entry->binding.keys,
            KeyBinding{
                .key = key,
                .modifiers = modifiers,
                .exactModifiers = exactModifiers,
            });
        resetEntryState(*entry);
    }
    return *this;
}

ActionMap& ActionMap::replaceKeyCombo(ActionId action, Key key, std::vector<Key> requiredKeys)
{
    if (Entry* entry = findEntry(action)) {
        entry->binding.keys.clear();
        appendUnique(
            entry->binding.keys,
            KeyBinding{
                .key = key,
                .requiredKeys = std::move(requiredKeys),
            });
        resetEntryState(*entry);
    }
    return *this;
}

ActionMap& ActionMap::replaceMouseButton(ActionId action, MouseButton button)
{
    if (Entry* entry = findEntry(action)) {
        entry->binding.mouseButtons.clear();
        appendUnique(entry->binding.mouseButtons, button);
        resetEntryState(*entry);
    }
    return *this;
}

ActionMap& ActionMap::replaceGamepadButton(ActionId action, GamepadButton button)
{
    if (Entry* entry = findEntry(action)) {
        entry->binding.gamepadButtons.clear();
        appendUnique(entry->binding.gamepadButtons, button);
        resetEntryState(*entry);
    }
    return *this;
}

ActionMap& ActionMap::replaceGamepadAxis(
    ActionId action,
    GamepadAxis axis,
    float deadzone,
    AxisDirection direction)
{
    if (Entry* entry = findEntry(action)) {
        entry->binding.gamepadAxes.clear();
        appendUnique(
            entry->binding.gamepadAxes,
            GamepadAxisBinding{
                .axis = axis,
                .deadzone = sanitizeDeadzone(deadzone),
                .direction = direction,
            });
        resetEntryState(*entry);
    }
    return *this;
}

void ActionMap::clearBindings(ActionId action)
{
    if (Entry* entry = findEntry(action)) {
        entry->binding.keys.clear();
        entry->binding.mouseButtons.clear();
        entry->binding.gamepadButtons.clear();
        entry->binding.gamepadAxes.clear();
        resetEntryState(*entry);
    }
}

void ActionMap::clear(ActionId action)
{
    if (!action) {
        return;
    }

    for (auto it = entries_.begin(); it != entries_.end(); ++it) {
        if (it->id == action) {
            entries_.erase(it);
            return;
        }
    }
}

void ActionMap::clearAll()
{
    entries_.clear();
    contexts_.clear();
}

void ActionMap::resetState() noexcept
{
    for (auto& entry : entries_) {
        resetEntryState(entry);
    }
}

ActionMap& ActionMap::setContext(ActionId action, std::string context)
{
    if (Entry* entry = findEntry(action)) {
        entry->binding.context = std::move(context);
        resetEntryState(*entry);
    }
    return *this;
}

void ActionMap::setContextEnabled(std::string context, bool enabled)
{
    if (context.empty()) {
        return;
    }

    if (ContextState* state = findContext(context)) {
        state->enabled = enabled;
        return;
    }

    contexts_.push_back(
        ContextState{
            .context = std::move(context),
            .enabled = enabled,
        });
}

bool ActionMap::isContextEnabled(std::string_view context) const noexcept
{
    if (context.empty()) {
        return true;
    }

    const ContextState* state = findContext(context);
    return !state || state->enabled;
}

void ActionMap::clearContextStates()
{
    contexts_.clear();
}

bool ActionMap::isDown(ActionId action) const
{
    const Entry* entry = findEntry(action);
    return entry && entry->down;
}

bool ActionMap::isPressed(ActionId action) const
{
    const Entry* entry = findEntry(action);
    return entry && entry->down && !entry->previousDown;
}

bool ActionMap::isReleased(ActionId action) const
{
    const Entry* entry = findEntry(action);
    return entry && !entry->down && entry->previousDown;
}

float ActionMap::axisValue(ActionId action) const
{
    const Entry* entry = findEntry(action);
    return entry ? entry->axisValue : 0.0f;
}

const ActionBinding* ActionMap::binding(ActionId action) const noexcept
{
    const Entry* entry = findEntry(action);
    return entry ? &entry->binding : nullptr;
}

ActionMap::Entry* ActionMap::findEntry(std::string_view action) noexcept
{
    for (auto& entry : entries_) {
        if (std::string_view{ entry.action } == action) {
            return &entry;
        }
    }

    return nullptr;
}

const ActionMap::Entry* ActionMap::findEntry(std::string_view action) const noexcept
{
    for (const auto& entry : entries_) {
        if (std::string_view{ entry.action } == action) {
            return &entry;
        }
    }

    return nullptr;
}

ActionMap::Entry* ActionMap::findEntry(ActionId action) noexcept
{
    if (!action) {
        return nullptr;
    }

    for (auto& entry : entries_) {
        if (entry.id == action) {
            return &entry;
        }
    }

    return nullptr;
}

const ActionMap::Entry* ActionMap::findEntry(ActionId action) const noexcept
{
    if (!action) {
        return nullptr;
    }

    for (const auto& entry : entries_) {
        if (entry.id == action) {
            return &entry;
        }
    }

    return nullptr;
}

ActionMap::ContextState* ActionMap::findContext(std::string_view context) noexcept
{
    for (auto& state : contexts_) {
        if (std::string_view{ state.context } == context) {
            return &state;
        }
    }

    return nullptr;
}

const ActionMap::ContextState* ActionMap::findContext(std::string_view context) const noexcept
{
    for (const auto& state : contexts_) {
        if (std::string_view{ state.context } == context) {
            return &state;
        }
    }

    return nullptr;
}

ActionMap::Entry& ActionMap::ensureEntry(std::string action)
{
    if (Entry* entry = findEntry(action)) {
        return *entry;
    }

    entries_.push_back(
        Entry{
            .id = nextActionId(),
            .action = std::move(action),
        });
    return entries_.back();
}

ActionId ActionMap::nextActionId() noexcept
{
    if (nextActionId_ == 0) {
        nextActionId_ = 1;
    }

    return ActionId{ nextActionId_++ };
}

void ActionMap::resetEntryState(Entry& entry) noexcept
{
    entry.down = false;
    entry.previousDown = false;
    entry.axisValue = 0.0f;
}

//----------------------------------------------------------------------------
//  Window Implementation
//----------------------------------------------------------------------------
struct Window::Impl
{
    explicit Impl(WindowDesc desc)
        : window(std::move(desc))
    {
    }

    backend::Window window;
};

Window::Window(std::unique_ptr<Impl> impl)
    : inputState_(impl->window.inputData()),
      impl_(std::move(impl))
{
}

Window::Window(Window&& other) noexcept
    : inputState_(std::move(other.inputState_)),
      impl_(std::move(other.impl_))
{
}

Window::~Window() = default;

NativeHandles Window::nativeHandles() const
{
    return impl_->window.nativeHandles();
}

VulkanHandle Window::createVulkanSurface(void* instance) const
{
    return impl_->window.createVulkanSurface(instance);
}

void Window::makeContextCurrent()
{
    impl_->window.makeContextCurrent();
}

void Window::swapBuffers()
{
    impl_->window.swapBuffers();
}

bool Window::shouldClose() const noexcept
{
    return impl_->window.shouldClose();
}

void Window::requestClose() noexcept
{
    impl_->window.requestClose();
}

std::span<const Event> Window::events() const noexcept
{
    return impl_->window.events();
}

const InputState& Window::input() const noexcept
{
    return inputState_;
}

void Window::setTitle(const std::string& title)
{
    impl_->window.setTitle(title);
}

std::string Window::title() const
{
    return impl_->window.title();
}

void Window::setSize(int width, int height)
{
    impl_->window.setSize(width, height);
}

void Window::setPosition(int x, int y)
{
    impl_->window.setPosition(x, y);
}

void Window::setSizeLimits(const SizeLimits& limits)
{
    impl_->window.setSizeLimits(limits);
}

void Window::clearSizeLimits()
{
    impl_->window.clearSizeLimits();
}

void Window::setAspectRatio(AspectRatio ratio)
{
    impl_->window.setAspectRatio(ratio);
}

void Window::clearAspectRatio()
{
    impl_->window.clearAspectRatio();
}

void Window::setResizable(bool resizable)
{
    impl_->window.setResizable(resizable);
}

void Window::setDecorated(bool decorated)
{
    impl_->window.setDecorated(decorated);
}

void Window::setFloating(bool floating)
{
    impl_->window.setFloating(floating);
}

void Window::setOpacity(float opacity)
{
    impl_->window.setOpacity(opacity);
}

void Window::setVSync(bool enabled)
{
    impl_->window.setVSync(enabled);
}

void Window::setCursorMode(CursorMode mode)
{
    impl_->window.setCursorMode(mode);
}

bool Window::setCursorShape(CursorShape shape)
{
    return impl_->window.setCursorShape(shape);
}

bool Window::setCursorImage(const ImageRgba& image, int hotX, int hotY)
{
    return impl_->window.setCursorImage(image, hotX, hotY);
}

void Window::clearCursor()
{
    impl_->window.clearCursor();
}

void Window::setMousePosition(double x, double y)
{
    impl_->window.setMousePosition(x, y);
}

bool Window::setRawMouseMotion(bool enabled)
{
    return impl_->window.setRawMouseMotion(enabled);
}

void Window::minimize()
{
    impl_->window.minimize();
}

void Window::maximize()
{
    impl_->window.maximize();
}

void Window::restore()
{
    impl_->window.restore();
}

void Window::setWindowMode(WindowMode mode, uint32_t monitorId, std::optional<VideoMode> videoMode)
{
    impl_->window.setWindowMode(mode, monitorId, videoMode);
}

bool Window::setIcon(const ImageRgba& image)
{
    return impl_->window.setIcon(std::span<const ImageRgba>{ &image, 1 });
}

bool Window::setIcons(std::span<const ImageRgba> images)
{
    return impl_->window.setIcon(images);
}

void Window::clearIcon()
{
    impl_->window.clearIcon();
}

void Window::requestAttention()
{
    impl_->window.requestAttention();
}

void Window::setFocus(bool focus) const noexcept
{
    impl_->window.setFocus(focus);
}

void Window::setVisible(bool visible) const noexcept
{
    impl_->window.setVisible(visible);
}

WindowPlacement Window::windowedPlacement() const noexcept
{
    return impl_->window.windowedPlacement();
}

void Window::setWindowedPlacement(const WindowPlacement& placement)
{
    impl_->window.setWindowedPlacement(placement);
}

std::pair<int, int> Window::size() const noexcept
{
    return impl_->window.size();
}

std::pair<int, int> Window::position() const noexcept
{
    return impl_->window.position();
}

std::pair<uint32_t, uint32_t> Window::framebufferSize() const noexcept
{
    return impl_->window.framebufferSize();
}

std::pair<float, float> Window::contentScale() const noexcept
{
    return impl_->window.contentScale();
}

DpiScale Window::dpiScale() const noexcept
{
    auto [x, y] = contentScale();
    return DpiScale{ .x = x, .y = y };
}

float Window::opacity() const noexcept
{
    return impl_->window.opacity();
}

CursorMode Window::cursorMode() const noexcept
{
    return impl_->window.cursorMode();
}

bool Window::isRawMouseMotionEnabled() const noexcept
{
    return impl_->window.isRawMouseMotionEnabled();
}

WindowMode Window::windowMode() const noexcept
{
    return impl_->window.windowMode();
}

bool Window::isResizable() const noexcept
{
    return impl_->window.isResizable();
}

bool Window::isDecorated() const noexcept
{
    return impl_->window.isDecorated();
}

bool Window::isFloating() const noexcept
{
    return impl_->window.isFloating();
}

bool Window::isMinimized() const noexcept
{
    return impl_->window.isMinimized();
}

bool Window::isMaximized() const noexcept
{
    return impl_->window.isMaximized();
}

bool Window::isFocused() const noexcept
{
    return impl_->window.isFocused();
}

bool Window::isVisible() const noexcept
{
    return impl_->window.isVisible();
}

//----------------------------------------------------------------------------
//  Window Builder Implementation
//----------------------------------------------------------------------------
struct WindowBuilder::Data
{
    GraphicsModeTag mode = NoneGraphicsModeTag();
    std::string title = "CppWindow";
    uint32_t width = 1280;
    uint32_t height = 720;
    std::optional<std::pair<int, int>> position;
    bool resizable = false;
    bool visible = true;
    bool decorated = true;
    bool focused = true;
    bool floating = false;
    std::optional<float> opacity;
    std::optional<SizeLimits> sizeLimits;
    std::optional<AspectRatio> aspectRatio;
    std::optional<CursorMode> cursorMode;
    std::optional<bool> rawMouseMotion;
    std::optional<bool> vSync;
    WindowMode windowMode = WindowMode::Windowed;
    uint32_t monitorId = 0;
    std::optional<VideoMode> videoMode;
};

WindowBuilder::WindowBuilder()
    : data_(std::make_unique<Data>()) {};

WindowBuilder::~WindowBuilder() = default;

WindowBuilder& WindowBuilder::title(std::string t)
{
    data_->title = std::move(t);
    return *this;
}

WindowBuilder& WindowBuilder::size(int w, int h)
{
    data_->width = w;
    data_->height = h;
    return *this;
}

WindowBuilder& WindowBuilder::position(int x, int y)
{
    data_->position = std::pair<int, int>{ x, y };
    return *this;
}

WindowBuilder& WindowBuilder::openGL(OpenGLConfig cfg)
{
    data_->mode = OpenGLGraphicsModeTag{
        .config = cfg,
    };
    return *this;
}

WindowBuilder& WindowBuilder::noGraphicsApi()
{
    data_->mode = NoneGraphicsModeTag{};
    return *this;
}

WindowBuilder& WindowBuilder::visible(bool visible)
{
    data_->visible = visible;
    if (!visible) {
        data_->focused = false;
    }
    return *this;
}

WindowBuilder& WindowBuilder::hidden()
{
    return visible(false);
}

WindowBuilder& WindowBuilder::resizable(bool resizable)
{
    data_->resizable = resizable;
    return *this;
}

WindowBuilder& WindowBuilder::focused(bool focused)
{
    data_->focused = focused;
    return *this;
}

WindowBuilder& WindowBuilder::decorated(bool decorated)
{
    data_->decorated = decorated;
    return *this;
}

WindowBuilder& WindowBuilder::borderless()
{
    return decorated(false);
}

WindowBuilder& WindowBuilder::floating(bool floating)
{
    data_->floating = floating;
    return *this;
}

WindowBuilder& WindowBuilder::opacity(float opacity)
{
    data_->opacity = opacity;
    return *this;
}

WindowBuilder& WindowBuilder::sizeLimits(const SizeLimits& limits)
{
    data_->sizeLimits = limits;
    return *this;
}

WindowBuilder& WindowBuilder::aspectRatio(AspectRatio ratio)
{
    data_->aspectRatio = ratio;
    return *this;
}

WindowBuilder& WindowBuilder::cursorMode(CursorMode mode)
{
    data_->cursorMode = mode;
    return *this;
}

WindowBuilder& WindowBuilder::rawMouseMotion(bool enabled)
{
    data_->rawMouseMotion = enabled;
    return *this;
}

WindowBuilder& WindowBuilder::vSync(bool enabled)
{
    data_->vSync = enabled;
    return *this;
}

WindowBuilder&
WindowBuilder::windowMode(WindowMode mode, uint32_t monitorId, std::optional<VideoMode> videoMode)
{
    data_->windowMode = mode;
    data_->monitorId = monitorId;
    data_->videoMode = videoMode;
    return *this;
}

Window WindowBuilder::build()
{
    WindowDesc desc{
        .mode = data_->mode,
        .title = data_->title,
        .width = data_->width,
        .height = data_->height,
        .position = data_->position,
        .resizable = data_->resizable,
        .visible = data_->visible,
        .decorated = data_->decorated,
        .focused = data_->focused,
        .floating = data_->floating,
        .opacity = data_->opacity,
        .sizeLimits = data_->sizeLimits,
        .aspectRatio = data_->aspectRatio,
        .cursorMode = data_->cursorMode,
        .rawMouseMotion = data_->rawMouseMotion,
        .vSync = data_->vSync,
        .windowMode = data_->windowMode,
        .monitorId = data_->monitorId,
        .videoMode = data_->videoMode,
    };
    auto impl = std::make_unique<Window::Impl>(std::move(desc));
    return Window(std::move(impl));
}

//----------------------------------------------------------------------------
//  Window Context Implemenation
//----------------------------------------------------------------------------
struct WindowContext::Impl
{
    backend::WindowContext context;
};

WindowContext& WindowContext::get()
{
    // init context
    static WindowContext instance;
    return instance;
}

WindowContext::WindowContext()
    : impl_(std::make_unique<Impl>())
{}

WindowContext::~WindowContext() = default;

void WindowContext::pollEvents() const noexcept
{
    impl_->context.pollEvents();
}

void WindowContext::waitEvents() const noexcept
{
    impl_->context.waitEvents();
}

void WindowContext::waitEventsTimeout(double timeoutSeconds) const noexcept
{
    impl_->context.waitEventsTimeout(timeoutSeconds);
}

void WindowContext::postEmptyEvent() const noexcept
{
    impl_->context.postEmptyEvent();
}

ProcLoader WindowContext::procLoader() const
{
    return impl_->context.procLoader();
}

bool WindowContext::isVulkanSupported() const
{
    return impl_->context.isVulkanSupported();
}

std::vector<std::string> WindowContext::requiredVulkanInstanceExtensions() const
{
    return impl_->context.requiredVulkanExtensions();
}

std::vector<MonitorInfo> WindowContext::monitors() const
{
    return impl_->context.monitors();
}

std::optional<MonitorInfo> WindowContext::primaryMonitor() const
{
    return impl_->context.primaryMonitor();
}

std::vector<VideoMode> WindowContext::videoModes(uint32_t monitorId) const
{
    return impl_->context.videoModes(monitorId);
}

std::pair<float, float> WindowContext::contentScale(uint32_t monitorId) const
{
    return impl_->context.contentScale(monitorId);
}

DpiScale WindowContext::dpiScale(uint32_t monitorId) const
{
    auto [x, y] = contentScale(monitorId);
    return DpiScale{ .x = x, .y = y };
}

std::vector<GamepadInfo> WindowContext::gamepads() const
{
    return impl_->context.gamepads();
}

std::optional<GamepadState> WindowContext::gamepadState(uint32_t gamepadId) const
{
    return impl_->context.gamepadState(gamepadId);
}

bool WindowContext::isRawMouseMotionSupported() const
{
    return impl_->context.isRawMouseMotionSupported();
}

std::optional<std::string> WindowContext::keyName(Key key, int scancode) const
{
    return impl_->context.keyName(key, scancode);
}

int WindowContext::keyScancode(Key key) const noexcept
{
    return impl_->context.keyScancode(key);
}

bool WindowContext::setClipboardText(std::string_view text) const
{
    return impl_->context.setClipboardText(text);
}

bool WindowContext::hasClipboardText() const
{
    const auto text = clipboardText();
    return text.has_value() && !text->empty();
}

std::optional<std::string> WindowContext::clipboardText() const
{
    return impl_->context.clipboardText();
}

}  // namespace cwin
