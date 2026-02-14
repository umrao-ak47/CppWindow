/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#include <cppwindow/cppwindow.hpp>

#include "backend/native_impl.hpp"

namespace cwin {

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

std::pair<double, double> InputState::getScrollDelta() const
{
    return state_->getScrollDelta();
}

//----------------------------------------------------------------------------
//  Window Implementation
//----------------------------------------------------------------------------
Window::Window(std::unique_ptr<NativeWindow> window)
    : window_(std::move(window)),
      inputState_(window->getInput())
{
}

Window::Window(Window&& other) noexcept
    : window_(std::move(other.window_)),
      inputState_(std::move(other.inputState_))
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

std::pair<uint32_t, uint32_t> Window::getFrameBufferSize() const noexcept
{
    return window_->getFrameBufferSize();
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

}  // namespace cwin