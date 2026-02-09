/*
 * Copyright (c) 2026 Your Name
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#include <glfw-vk/Context.hpp>
#include <glfw-vk/Window.hpp>

#include "internal/GlfwIncludes.hpp"
#include "internal/InputState.hpp"

// Manually forward declare the Vulkan types GLFW needs
// This prevents needing <vulkan.h> entirely.
typedef struct VkInstance_T* VkInstance;
typedef uint64_t VkSurfaceKHR;
typedef int VkResult;

// Tell the compiler that this function exists externally (it's in the GLFW lib)
extern "C" VkResult glfwCreateWindowSurface(
    VkInstance instance,
    GLFWwindow* window,
    const void* allocator,
    VkSurfaceKHR* surface);

namespace glfwvk {

namespace {

struct GLFWwindowDeleter
{
    void operator()(GLFWwindow* window) const noexcept
    {
        if (window) {
            glfwDestroyWindow(window);
        }
    }
};

using UniqueGLFWwindow = std::unique_ptr<GLFWwindow, GLFWwindowDeleter>;

}  // namespace

struct Window::Impl
{
    InputState inputState;
    UniqueGLFWwindow windowHandle = nullptr;

    Impl(const WindowDesc& desc)
    {
        // Window hints
        glfwWindowHint(GLFW_RESIZABLE, desc.resizable ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_VISIBLE, desc.visible ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_DECORATED, desc.decorated ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_FOCUSED, desc.focused ? GLFW_TRUE : GLFW_FALSE);
        // set no api for vulkan
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        windowHandle.reset(glfwCreateWindow(
            desc.width,
            desc.height,
            desc.title.c_str(),
            nullptr,  // monitor (fullscreen later)
            nullptr   // share
            ));

        if (!windowHandle) {
            throw GLFWException("Failed to create window");
        }
    }

    GLFWwindow* window()
    {
        return windowHandle.get();
    }

    InputState& getInput()
    {
        return inputState;
    }
};

Window::Window(const WindowDesc& desc)
    : m_impl(std::make_unique<Impl>(desc))
{
    // set data and register callbacks
    glfwSetWindowUserPointer(m_impl->window(), m_impl.get());
    registerCallbacks();
}

Window::Window(Window&& other) noexcept
    : m_impl(std::move(other.m_impl))
{
}

Window::~Window() = default;

Window& Window::operator=(Window&& other) noexcept
{
    if (this != &other) {
        m_impl = std::move(other.m_impl);
    }
    return *this;
}

void* Window::nativeHandle() const noexcept
{
    return static_cast<void*>(m_impl->window());
}

Window::SurfaceHandle Window::createSurface(void* instance) const
{
    VkSurfaceKHR surface = 0;

    // 3. Call the function directly
    // The linker will find this inside the glfw3 library you are linking against
    VkResult result = glfwCreateWindowSurface(
        static_cast<VkInstance>(instance),
        m_impl->window(),
        nullptr,
        &surface);
    // VK_SUCCESS is 0
    if (result != 0) {
        throw GLFWException("Failed to create window surface via GLFW");
    }
    return static_cast<SurfaceHandle>(surface);
}

bool Window::shouldClose() const noexcept
{
    return glfwWindowShouldClose(m_impl->window()) == GLFW_TRUE;
}

void Window::requestClose() noexcept
{
    glfwSetWindowShouldClose(m_impl->window(), GLFW_TRUE);
}

void Window::pollEvents() const noexcept
{
    glfwPollEvents();
}

std::optional<Event> Window::pollEvent() const noexcept
{
    return m_impl->getInput().popEvent();
}

const Input& Window::getInput() const noexcept
{
    return m_impl->getInput();
}

void Window::setTitle(const std::string& title)
{
    glfwSetWindowTitle(m_impl->window(), title.c_str());
}

void Window::setSize(int width, int height)
{
    glfwSetWindowSize(m_impl->window(), width, height);
}

std::pair<int, int> Window::getSize() const noexcept
{
    int width, height;
    glfwGetWindowSize(m_impl->window(), &width, &height);
    return {
        width,
        height,
    };
}

std::pair<uint32_t, uint32_t> Window::getFrameBufferSize() const noexcept
{
    int width, height;
    glfwGetFramebufferSize(m_impl->window(), &width, &height);
    return {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height),
    };
}

bool Window::isFocused() const noexcept
{
    return glfwGetWindowAttrib(m_impl->window(), GLFW_FOCUSED) == GLFW_TRUE;
}

bool Window::isVisible() const noexcept
{
    return glfwGetWindowAttrib(m_impl->window(), GLFW_VISIBLE) == GLFW_TRUE;
}

void Window::registerCallbacks()
{
    GLFWwindow* window = m_impl->window();
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* win, int w, int h) {
        auto* self = static_cast<Window::Impl*>(glfwGetWindowUserPointer(win));
        uint32_t width = static_cast<uint32_t>(w);
        uint32_t height = static_cast<uint32_t>(h);
        self->getInput().handleFrameBufferResizeEvent(width, height);
    });

    glfwSetWindowCloseCallback(window, [](GLFWwindow* window) {
        auto* self = static_cast<Window::Impl*>(glfwGetWindowUserPointer(window));
        self->getInput().handleCloseEvent();
    });

    glfwSetWindowFocusCallback(window, [](GLFWwindow* window, int focused) {
        auto* self = static_cast<Window::Impl*>(glfwGetWindowUserPointer(window));
        self->getInput().handleFocusEvent(focused);
    });

    glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        auto* self = static_cast<Window::Impl*>(glfwGetWindowUserPointer(window));
        self->getInput().handleResizeEvent(width, height);
    });

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto* self = static_cast<Window::Impl*>(glfwGetWindowUserPointer(window));
        self->getInput().handleKeyEvent(key, scancode, action, mods);
    });

    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
        auto* self = static_cast<Window::Impl*>(glfwGetWindowUserPointer(window));
        self->getInput().handleMouseEvent(button, action, mods);
    });

    glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
        auto* self = static_cast<Window::Impl*>(glfwGetWindowUserPointer(window));
        self->getInput().handleScrollEvent(xoffset, yoffset);
    });

    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
        auto* self = static_cast<Window::Impl*>(glfwGetWindowUserPointer(window));
        self->getInput().handleCursorPosEvent(xpos, ypos);
    });
}

}  // namespace glfwvk