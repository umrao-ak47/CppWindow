/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#include <cppwindow/cppwindow.hpp>

#include "backend/native_impl.hpp"

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

std::span<Event> Window::events() const noexcept
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

float Window::getOpacity() const noexcept
{
    return window_->getOpacity();
}

CursorMode Window::getCursorMode() const noexcept
{
    return window_->getCursorMode();
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
    bool resizable = false;
    bool visible = true;
    bool decorated = true;
    bool focused = true;
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

WindowBuilder& WindowBuilder::hidden()
{
    data_->visible = false;
    data_->focused = false;
    return *this;
}

WindowBuilder& WindowBuilder::resizable()
{
    data_->resizable = true;
    return *this;
}

WindowBuilder& WindowBuilder::borderless()
{
    data_->decorated = false;
    return *this;
}

Window WindowBuilder::build()
{
    WindowDesc desc{
        .mode = data_->mode,
        .title = data_->title,
        .width = data_->width,
        .height = data_->height,
        .resizable = data_->resizable,
        .visible = data_->visible,
        .decorated = data_->decorated,
        .focused = data_->focused,
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

std::vector<GamepadInfo> WindowContext::getGamepads() const
{
    return context_->getGamepads();
}

std::optional<GamepadState> WindowContext::getGamepadState(uint32_t gamepadId) const
{
    return context_->getGamepadState(gamepadId);
}

void WindowContext::setClipboardText(const std::string& text) const
{
    context_->setClipboardText(text);
}

std::string WindowContext::getClipboardText() const
{
    return context_->getClipboardText();
}

}  // namespace cwin
