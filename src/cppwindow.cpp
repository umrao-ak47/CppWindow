/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#include <cppwindow/cppwindow.hpp>

#include <algorithm>
#include <cmath>
#include <thread>

#include "backend/native_impl.hpp"

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

float GamepadState::getAxis(GamepadAxis axis) const noexcept
{
    if (!isValidGamepadAxis(axis)) {
        return 0.0f;
    }

    return axes[static_cast<size_t>(axis)];
}

//----------------------------------------------------------------------------
//  Input State Implemenation
//----------------------------------------------------------------------------
InputState::InputState(const NativeInputState* state)
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

std::pair<double, double> InputState::getMousePosition() const
{
    return state_->getMousePosition();
}

std::pair<double, double> InputState::getMouseDelta() const
{
    return state_->getMouseDelta();
}

std::pair<double, double> InputState::getScrollDelta() const
{
    return state_->getScrollDelta();
}

bool InputState::isMouseInside() const
{
    return state_->isMouseInside();
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

ActionMap& ActionMap::bindKey(std::string action, Key key, Modifiers modifiers, bool exactModifiers)
{
    appendUnique(
        getOrCreateEntry(std::move(action)).binding.keys,
        KeyBinding{
            .key = key,
            .modifiers = modifiers,
            .exactModifiers = exactModifiers,
        });
    return *this;
}

ActionMap& ActionMap::bindKeyChord(std::string action, Key key, std::vector<Key> requiredKeys)
{
    appendUnique(
        getOrCreateEntry(std::move(action)).binding.keys,
        KeyBinding{
            .key = key,
            .requiredKeys = std::move(requiredKeys),
        });
    return *this;
}

ActionMap& ActionMap::bindMouseButton(std::string action, MouseButton button)
{
    appendUnique(getOrCreateEntry(std::move(action)).binding.mouseButtons, button);
    return *this;
}

ActionMap& ActionMap::bindGamepadButton(std::string action, GamepadButton button)
{
    appendUnique(getOrCreateEntry(std::move(action)).binding.gamepadButtons, button);
    return *this;
}

ActionMap& ActionMap::bindGamepadAxis(
    std::string action,
    GamepadAxis axis,
    float deadzone,
    ActionAxisDirection direction)
{
    appendUnique(
        getOrCreateEntry(std::move(action)).binding.gamepadAxes,
        GamepadAxisBinding{
            .axis = axis,
            .deadzone = sanitizeDeadzone(deadzone),
            .direction = direction,
        });
    return *this;
}

ActionMap&
ActionMap::replaceKey(std::string action, Key key, Modifiers modifiers, bool exactModifiers)
{
    Entry& entry = getOrCreateEntry(std::move(action));
    entry.binding.keys.clear();
    appendUnique(
        entry.binding.keys,
        KeyBinding{
            .key = key,
            .modifiers = modifiers,
            .exactModifiers = exactModifiers,
        });
    resetEntryState(entry);
    return *this;
}

ActionMap& ActionMap::replaceKeyChord(std::string action, Key key, std::vector<Key> requiredKeys)
{
    Entry& entry = getOrCreateEntry(std::move(action));
    entry.binding.keys.clear();
    appendUnique(
        entry.binding.keys,
        KeyBinding{
            .key = key,
            .requiredKeys = std::move(requiredKeys),
        });
    resetEntryState(entry);
    return *this;
}

ActionMap& ActionMap::replaceMouseButton(std::string action, MouseButton button)
{
    Entry& entry = getOrCreateEntry(std::move(action));
    entry.binding.mouseButtons.clear();
    appendUnique(entry.binding.mouseButtons, button);
    resetEntryState(entry);
    return *this;
}

ActionMap& ActionMap::replaceGamepadButton(std::string action, GamepadButton button)
{
    Entry& entry = getOrCreateEntry(std::move(action));
    entry.binding.gamepadButtons.clear();
    appendUnique(entry.binding.gamepadButtons, button);
    resetEntryState(entry);
    return *this;
}

ActionMap& ActionMap::replaceGamepadAxis(
    std::string action,
    GamepadAxis axis,
    float deadzone,
    ActionAxisDirection direction)
{
    Entry& entry = getOrCreateEntry(std::move(action));
    entry.binding.gamepadAxes.clear();
    appendUnique(
        entry.binding.gamepadAxes,
        GamepadAxisBinding{
            .axis = axis,
            .deadzone = sanitizeDeadzone(deadzone),
            .direction = direction,
        });
    resetEntryState(entry);
    return *this;
}

void ActionMap::clearBindings(const std::string& action)
{
    if (Entry* entry = findEntry(action)) {
        entry->binding.keys.clear();
        entry->binding.mouseButtons.clear();
        entry->binding.gamepadButtons.clear();
        entry->binding.gamepadAxes.clear();
        resetEntryState(*entry);
    }
}

void ActionMap::clear(const std::string& action)
{
    for (auto it = entries_.begin(); it != entries_.end(); ++it) {
        if (it->action == action) {
            entries_.erase(it);
            return;
        }
    }
}

void ActionMap::clearAll()
{
    entries_.clear();
    groups_.clear();
}

void ActionMap::resetState() noexcept
{
    for (auto& entry : entries_) {
        resetEntryState(entry);
    }
}

ActionMap& ActionMap::setGroup(std::string action, std::string group)
{
    Entry& entry = getOrCreateEntry(std::move(action));
    entry.binding.group = std::move(group);
    resetEntryState(entry);
    return *this;
}

void ActionMap::setGroupEnabled(std::string group, bool enabled)
{
    if (group.empty()) {
        return;
    }

    if (GroupState* state = findGroup(group)) {
        state->enabled = enabled;
        return;
    }

    groups_.push_back(
        GroupState{
            .group = std::move(group),
            .enabled = enabled,
        });
}

bool ActionMap::isGroupEnabled(const std::string& group) const noexcept
{
    if (group.empty()) {
        return true;
    }

    const GroupState* state = findGroup(group);
    return !state || state->enabled;
}

void ActionMap::clearGroupStates()
{
    groups_.clear();
}

bool ActionMap::isDown(const std::string& action) const
{
    const Entry* entry = findEntry(action);
    return entry && entry->down;
}

bool ActionMap::isPressed(const std::string& action) const
{
    const Entry* entry = findEntry(action);
    return entry && entry->down && !entry->previousDown;
}

bool ActionMap::isReleased(const std::string& action) const
{
    const Entry* entry = findEntry(action);
    return entry && !entry->down && entry->previousDown;
}

float ActionMap::getAxis(const std::string& action) const
{
    const Entry* entry = findEntry(action);
    return entry ? entry->axisValue : 0.0f;
}

const ActionBinding* ActionMap::getBinding(const std::string& action) const noexcept
{
    const Entry* entry = findEntry(action);
    return entry ? &entry->binding : nullptr;
}

ActionMap::Entry* ActionMap::findEntry(const std::string& action) noexcept
{
    for (auto& entry : entries_) {
        if (entry.action == action) {
            return &entry;
        }
    }

    return nullptr;
}

const ActionMap::Entry* ActionMap::findEntry(const std::string& action) const noexcept
{
    for (const auto& entry : entries_) {
        if (entry.action == action) {
            return &entry;
        }
    }

    return nullptr;
}

ActionMap::GroupState* ActionMap::findGroup(const std::string& group) noexcept
{
    for (auto& state : groups_) {
        if (state.group == group) {
            return &state;
        }
    }

    return nullptr;
}

const ActionMap::GroupState* ActionMap::findGroup(const std::string& group) const noexcept
{
    for (const auto& state : groups_) {
        if (state.group == group) {
            return &state;
        }
    }

    return nullptr;
}

ActionMap::Entry& ActionMap::getOrCreateEntry(std::string action)
{
    if (Entry* entry = findEntry(action)) {
        return *entry;
    }

    entries_.push_back(
        Entry{
            .action = std::move(action),
        });
    return entries_.back();
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
Window::Window(std::unique_ptr<NativeWindow> window)
    : inputState_(window->getInput()),
      window_(std::move(window))
{
}

Window::Window(Window&& other) noexcept
    : inputState_(std::move(other.inputState_)),
      window_(std::move(other.window_))
{
}

Window::~Window() = default;

NativeHandles Window::getNativeHandles() const
{
    return window_->getNativeHandles();
}

VulkanHandle Window::createVulkanSurface(void* instance) const
{
    return window_->createVulkanSurface(instance);
}

void Window::makeContextCurrent()
{
    window_->makeContextCurrent();
}

void Window::swapBuffers()
{
    window_->swapBuffers();
}

bool Window::shouldClose() const noexcept
{
    return window_->shouldClose();
}

void Window::requestClose() noexcept
{
    window_->requestClose();
}

std::span<const Event> Window::events() const noexcept
{
    return window_->events();
}

const InputState& Window::getInput() const noexcept
{
    return inputState_;
}

void Window::setTitle(const std::string& title)
{
    window_->setTitle(title);
}

void Window::setSize(int width, int height)
{
    window_->setSize(width, height);
}

void Window::setPosition(int x, int y)
{
    window_->setPosition(x, y);
}

void Window::setSizeLimits(const SizeLimits& limits)
{
    window_->setSizeLimits(limits);
}

void Window::clearSizeLimits()
{
    window_->clearSizeLimits();
}

void Window::setAspectRatio(AspectRatio ratio)
{
    window_->setAspectRatio(ratio);
}

void Window::clearAspectRatio()
{
    window_->clearAspectRatio();
}

void Window::setResizable(bool resizable)
{
    window_->setResizable(resizable);
}

void Window::setDecorated(bool decorated)
{
    window_->setDecorated(decorated);
}

void Window::setFloating(bool floating)
{
    window_->setFloating(floating);
}

void Window::setOpacity(float opacity)
{
    window_->setOpacity(opacity);
}

void Window::setVSync(bool enabled)
{
    window_->setVSync(enabled);
}

void Window::setCursorMode(CursorMode mode)
{
    window_->setCursorMode(mode);
}

bool Window::setCursorShape(CursorShape shape)
{
    return window_->setCursorShape(shape);
}

bool Window::setCursorImage(const ImageRgba& image, int hotX, int hotY)
{
    return window_->setCursorImage(image, hotX, hotY);
}

void Window::clearCursor()
{
    window_->clearCursor();
}

void Window::setMousePosition(double x, double y)
{
    window_->setMousePosition(x, y);
}

bool Window::setRawMouseMotion(bool enabled)
{
    return window_->setRawMouseMotion(enabled);
}

void Window::minimize()
{
    window_->minimize();
}

void Window::maximize()
{
    window_->maximize();
}

void Window::restore()
{
    window_->restore();
}

void Window::setWindowMode(WindowMode mode, uint32_t monitorId, std::optional<VideoMode> videoMode)
{
    window_->setWindowMode(mode, monitorId, videoMode);
}

bool Window::setIcon(const ImageRgba& image)
{
    return window_->setIcon(std::span<const ImageRgba>{ &image, 1 });
}

bool Window::setIcons(std::span<const ImageRgba> images)
{
    return window_->setIcon(images);
}

void Window::clearIcon()
{
    window_->clearIcon();
}

void Window::requestAttention()
{
    window_->requestAttention();
}

void Window::setFocus(bool focus) const noexcept
{
    window_->setFocus(focus);
}

void Window::setVisible(bool visible) const noexcept
{
    window_->setVisible(visible);
}

std::pair<int, int> Window::getSize() const noexcept
{
    return window_->getSize();
}

std::pair<int, int> Window::getPosition() const noexcept
{
    return window_->getPosition();
}

std::pair<uint32_t, uint32_t> Window::getFrameBufferSize() const noexcept
{
    return window_->getFrameBufferSize();
}

std::pair<float, float> Window::getContentScale() const noexcept
{
    return window_->getContentScale();
}

DpiScale Window::getDpiScale() const noexcept
{
    auto [x, y] = getContentScale();
    return DpiScale{ .x = x, .y = y };
}

float Window::getOpacity() const noexcept
{
    return window_->getOpacity();
}

CursorMode Window::getCursorMode() const noexcept
{
    return window_->getCursorMode();
}

bool Window::isRawMouseMotionEnabled() const noexcept
{
    return window_->isRawMouseMotionEnabled();
}

WindowMode Window::getWindowMode() const noexcept
{
    return window_->getWindowMode();
}

bool Window::isFocused() const noexcept
{
    return window_->isFocused();
}

bool Window::isVisible() const noexcept
{
    return window_->isVisible();
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

WindowBuilder& WindowBuilder::noAPI()
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
    auto native = factory::createNativeWindow(std::move(desc));
    return Window(std::move(native));
}

//----------------------------------------------------------------------------
//  Window Context Implemenation
//----------------------------------------------------------------------------
WindowContext& WindowContext::Get()
{
    // init context
    static WindowContext instance;
    return instance;
}

WindowContext::WindowContext()
{
    context_.reset();
    context_ = factory::createNativeContext();
}

WindowContext::~WindowContext() = default;

void WindowContext::pollEvents() const noexcept
{
    context_->pollEvents();
}

void WindowContext::waitEvents() const noexcept
{
    context_->waitEvents();
}

void WindowContext::waitEventsTimeout(double timeoutSeconds) const noexcept
{
    context_->waitEventsTimeout(timeoutSeconds);
}

void WindowContext::postEmptyEvent() const noexcept
{
    context_->postEmptyEvent();
}

ProcLoader WindowContext::getProcLoader() const
{
    return context_->getProcLoader();
}

bool WindowContext::isVulkanSupported() const
{
    return context_->isVulkanSupported();
}

std::vector<std::string> WindowContext::getRequiredGlfwVulkanExtensions() const
{
    return context_->getRequiredVulkanExtensions();
}

std::vector<MonitorInfo> WindowContext::getMonitors() const
{
    return context_->getMonitors();
}

std::optional<MonitorInfo> WindowContext::getPrimaryMonitor() const
{
    return context_->getPrimaryMonitor();
}

std::vector<VideoMode> WindowContext::getVideoModes(uint32_t monitorId) const
{
    return context_->getVideoModes(monitorId);
}

std::pair<float, float> WindowContext::getContentScale(uint32_t monitorId) const
{
    return context_->getContentScale(monitorId);
}

DpiScale WindowContext::getDpiScale(uint32_t monitorId) const
{
    auto [x, y] = getContentScale(monitorId);
    return DpiScale{ .x = x, .y = y };
}

std::vector<GamepadInfo> WindowContext::getGamepads() const
{
    return context_->getGamepads();
}

std::optional<GamepadState> WindowContext::getGamepadState(uint32_t gamepadId) const
{
    return context_->getGamepadState(gamepadId);
}

bool WindowContext::isRawMouseMotionSupported() const
{
    return context_->isRawMouseMotionSupported();
}

bool WindowContext::setClipboardText(const std::string& text) const
{
    return context_->setClipboardText(text);
}

bool WindowContext::hasClipboardText() const
{
    const auto text = tryGetClipboardText();
    return text.has_value() && !text->empty();
}

std::string WindowContext::getClipboardText() const
{
    return context_->getClipboardText();
}

std::optional<std::string> WindowContext::tryGetClipboardText() const
{
    return context_->tryGetClipboardText();
}

}  // namespace cwin
