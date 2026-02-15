/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#include "glfw_impl.hpp"

#if defined(CPPWINDOW_PLATFORM_WINDOWS)
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#elif defined(CPPWINDOW_PLATFORM_MACOS)
#define GLFW_EXPOSE_NATIVE_COCOA
#define GLFW_EXPOSE_NATIVE_NSGL
#elif defined(CPPWINDOW_PLATFORM_LINUX)
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_WAYLAND
#define GLFW_EXPOSE_NATIVE_GLX
#else
#error "Unknow Platform"
#endif
#include <GLFW/glfw3native.h>

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

namespace cwin {

//----------------------------------------------------------------------------
//  GLFW Input Mapping Implementation
//----------------------------------------------------------------------------
namespace {

static constexpr KeyMapLookup KeyMap(
    {
        /* Printable keys */
        { Key::Space, GLFW_KEY_SPACE },
        { Key::Apostrophe, GLFW_KEY_APOSTROPHE },
        { Key::Comma, GLFW_KEY_COMMA },
        { Key::Minus, GLFW_KEY_MINUS },
        { Key::Period, GLFW_KEY_PERIOD },
        { Key::Slash, GLFW_KEY_SLASH },

        { Key::Num0, GLFW_KEY_0 },
        { Key::Num1, GLFW_KEY_1 },
        { Key::Num2, GLFW_KEY_2 },
        { Key::Num3, GLFW_KEY_3 },
        { Key::Num4, GLFW_KEY_4 },
        { Key::Num5, GLFW_KEY_5 },
        { Key::Num6, GLFW_KEY_6 },
        { Key::Num7, GLFW_KEY_7 },
        { Key::Num8, GLFW_KEY_8 },
        { Key::Num9, GLFW_KEY_9 },

        { Key::Semicolon, GLFW_KEY_SEMICOLON },
        { Key::Equal, GLFW_KEY_EQUAL },

        { Key::A, GLFW_KEY_A },
        { Key::B, GLFW_KEY_B },
        { Key::C, GLFW_KEY_C },
        { Key::D, GLFW_KEY_D },
        { Key::E, GLFW_KEY_E },
        { Key::F, GLFW_KEY_F },
        { Key::G, GLFW_KEY_G },
        { Key::H, GLFW_KEY_H },
        { Key::I, GLFW_KEY_I },
        { Key::J, GLFW_KEY_J },
        { Key::K, GLFW_KEY_K },
        { Key::L, GLFW_KEY_L },
        { Key::M, GLFW_KEY_M },
        { Key::N, GLFW_KEY_N },
        { Key::O, GLFW_KEY_O },
        { Key::P, GLFW_KEY_P },
        { Key::Q, GLFW_KEY_Q },
        { Key::R, GLFW_KEY_R },
        { Key::S, GLFW_KEY_S },
        { Key::T, GLFW_KEY_T },
        { Key::U, GLFW_KEY_U },
        { Key::V, GLFW_KEY_V },
        { Key::W, GLFW_KEY_W },
        { Key::X, GLFW_KEY_X },
        { Key::Y, GLFW_KEY_Y },
        { Key::Z, GLFW_KEY_Z },

        { Key::LBracket, GLFW_KEY_LEFT_BRACKET },
        { Key::Backslash, GLFW_KEY_BACKSLASH },
        { Key::RBracket, GLFW_KEY_RIGHT_BRACKET },
        { Key::Grave, GLFW_KEY_GRAVE_ACCENT },
        { Key::World1, GLFW_KEY_WORLD_1 },
        { Key::World2, GLFW_KEY_WORLD_2 },

        /* Function keys */
        { Key::Escape, GLFW_KEY_ESCAPE },
        { Key::Enter, GLFW_KEY_ENTER },
        { Key::Tab, GLFW_KEY_TAB },
        { Key::Backspace, GLFW_KEY_BACKSPACE },
        { Key::Insert, GLFW_KEY_INSERT },
        { Key::Delete, GLFW_KEY_DELETE },

        { Key::Right, GLFW_KEY_RIGHT },
        { Key::Left, GLFW_KEY_LEFT },
        { Key::Down, GLFW_KEY_DOWN },
        { Key::Up, GLFW_KEY_UP },

        { Key::PageUp, GLFW_KEY_PAGE_UP },
        { Key::PageDown, GLFW_KEY_PAGE_DOWN },
        { Key::Home, GLFW_KEY_HOME },
        { Key::End, GLFW_KEY_END },
        { Key::CapsLock, GLFW_KEY_CAPS_LOCK },
        { Key::ScrollLock, GLFW_KEY_SCROLL_LOCK },
        { Key::NumLock, GLFW_KEY_NUM_LOCK },
        { Key::PrintScreen, GLFW_KEY_PRINT_SCREEN },
        { Key::Pause, GLFW_KEY_PAUSE },

        /* Function keys */
        { Key::F1, GLFW_KEY_F1 },
        { Key::F2, GLFW_KEY_F2 },
        { Key::F3, GLFW_KEY_F3 },
        { Key::F4, GLFW_KEY_F4 },
        { Key::F5, GLFW_KEY_F5 },
        { Key::F6, GLFW_KEY_F6 },
        { Key::F7, GLFW_KEY_F7 },
        { Key::F8, GLFW_KEY_F8 },
        { Key::F9, GLFW_KEY_F9 },
        { Key::F10, GLFW_KEY_F10 },
        { Key::F11, GLFW_KEY_F11 },
        { Key::F12, GLFW_KEY_F12 },
        { Key::F13, GLFW_KEY_F13 },
        { Key::F14, GLFW_KEY_F14 },
        { Key::F15, GLFW_KEY_F15 },
        { Key::F16, GLFW_KEY_F16 },
        { Key::F17, GLFW_KEY_F17 },
        { Key::F18, GLFW_KEY_F18 },
        { Key::F19, GLFW_KEY_F19 },
        { Key::F20, GLFW_KEY_F20 },
        { Key::F21, GLFW_KEY_F21 },
        { Key::F22, GLFW_KEY_F22 },
        { Key::F23, GLFW_KEY_F23 },
        { Key::F24, GLFW_KEY_F24 },
        { Key::F25, GLFW_KEY_F25 },

        /* Numpad Keys */
        { Key::Numpad0, GLFW_KEY_KP_0 },
        { Key::Numpad1, GLFW_KEY_KP_1 },
        { Key::Numpad2, GLFW_KEY_KP_2 },
        { Key::Numpad3, GLFW_KEY_KP_3 },
        { Key::Numpad4, GLFW_KEY_KP_4 },
        { Key::Numpad5, GLFW_KEY_KP_5 },
        { Key::Numpad6, GLFW_KEY_KP_6 },
        { Key::Numpad7, GLFW_KEY_KP_7 },
        { Key::Numpad8, GLFW_KEY_KP_8 },
        { Key::Numpad9, GLFW_KEY_KP_9 },
        { Key::NumpadDecimal, GLFW_KEY_KP_DECIMAL },
        { Key::NumpadDivide, GLFW_KEY_KP_DIVIDE },
        { Key::NumpadMultiply, GLFW_KEY_KP_MULTIPLY },
        { Key::NumpadSubtract, GLFW_KEY_KP_SUBTRACT },
        { Key::NumpadAdd, GLFW_KEY_KP_ADD },
        { Key::NumpadEnter, GLFW_KEY_KP_ENTER },
        { Key::NumpadEqual, GLFW_KEY_KP_EQUAL },

        { Key::LShift, GLFW_KEY_LEFT_SHIFT },
        { Key::LControl, GLFW_KEY_LEFT_CONTROL },
        { Key::LAlt, GLFW_KEY_LEFT_ALT },
        { Key::LSuper, GLFW_KEY_LEFT_SUPER },
        { Key::RShift, GLFW_KEY_RIGHT_SHIFT },
        { Key::RControl, GLFW_KEY_RIGHT_CONTROL },
        { Key::RAlt, GLFW_KEY_RIGHT_ALT },
        { Key::RSuper, GLFW_KEY_RIGHT_SUPER },
        { Key::Menu, GLFW_KEY_MENU },
    });

static constexpr MouseMapLookup MouseMap(
    {
        { MouseButton::Left, GLFW_MOUSE_BUTTON_LEFT },
        { MouseButton::Right, GLFW_MOUSE_BUTTON_RIGHT },
        { MouseButton::Middle, GLFW_MOUSE_BUTTON_MIDDLE },
        { MouseButton::Button4, GLFW_MOUSE_BUTTON_4 },
        { MouseButton::Button5, GLFW_MOUSE_BUTTON_5 },
        { MouseButton::Button6, GLFW_MOUSE_BUTTON_6 },
        { MouseButton::Button7, GLFW_MOUSE_BUTTON_7 },
        { MouseButton::Button8, GLFW_MOUSE_BUTTON_8 },
    });

}  // namespace

namespace inputmap {

int toGlfwKey(Key k)
{
    return KeyMap.toBackend(k);
}

Key toKey(int k)
{
    return KeyMap.toWrapper(k);
}

// Two-way Mouse
int toGlfwMouseButton(MouseButton b)
{
    return MouseMap.toBackend(b);
}

MouseButton toMouseButton(int b)
{
    return MouseMap.toWrapper(b);
}

}  // namespace inputmap

//----------------------------------------------------------------------------
//  GLFW Input State Implementation
//----------------------------------------------------------------------------

void GLFWInputState::handleEvent(const Event& event)
{
    event.visit([&](auto&& event) {
        using T = std::decay_t<decltype(event)>;
        if constexpr (std::is_same_v<T, Event::KeyPressed>) {
            keyStates_.set(static_cast<size_t>(event.key));
        } else if constexpr (std::is_same_v<T, Event::KeyReleased>) {
            keyStates_.reset(static_cast<size_t>(event.key));
        } else if constexpr (std::is_same_v<T, Event::MouseButtonPressed>) {
            mouseStates_.set(static_cast<size_t>(event.button));
        } else if constexpr (std::is_same_v<T, Event::MouseButtonReleased>) {
            mouseStates_.reset(static_cast<size_t>(event.button));
        } else if constexpr (std::is_same_v<T, Event::MouseWheelScrolled>) {
            scrollDeltaX_ += event.deltaX;
            scrollDeltaY_ += event.deltaY;
        } else if constexpr (std::is_same_v<T, Event::MouseMoved>) {
            mousePosX_ = event.posX;
            mousePosY_ = event.posY;
        } else if constexpr (std::is_same_v<T, Event::FocusLost>) {
            keyStates_.reset();
            mouseStates_.reset();
        }
    });
}

bool GLFWInputState::isKeyDown(Key key) const
{
    return keyStates_.test(static_cast<size_t>(key));
}

bool GLFWInputState::isKeyPressed(Key key) const
{
    size_t idx = static_cast<size_t>(key);
    return keyStates_.test(idx) && !prevKeyStates_.test(idx);
}

bool GLFWInputState::isKeyReleased(Key key) const
{
    size_t idx = static_cast<size_t>(key);
    return !keyStates_.test(idx) && prevKeyStates_.test(idx);
}

bool GLFWInputState::isMouseButtonDown(MouseButton button) const
{
    return mouseStates_.test(static_cast<size_t>(button));
}

bool GLFWInputState::isMouseButtonPressed(MouseButton button) const
{
    size_t idx = static_cast<size_t>(button);
    return mouseStates_.test(idx) && !prevMouseStates_.test(idx);
}

bool GLFWInputState::isMouseButtonReleased(MouseButton button) const
{
    size_t idx = static_cast<size_t>(button);
    return !mouseStates_.test(idx) && prevMouseStates_.test(idx);
}

std::pair<double, double> GLFWInputState::getMousePosition() const
{
    return { mousePosX_, mousePosY_ };
}

std::pair<double, double> GLFWInputState::getScrollDelta() const
{
    return { scrollDeltaX_, scrollDeltaY_ };
}

void GLFWInputState::reset()
{
    prevKeyStates_ = keyStates_;
    prevMouseStates_ = mouseStates_;
    scrollDeltaX_ = 0;
    scrollDeltaY_ = 0;
}

//----------------------------------------------------------------------------
//  GLFW Window Registry Implementation
//----------------------------------------------------------------------------
namespace {

// store all refs to windows storage so we can clean it
// per frame
WindowStorageRegistry<WindowStorage> g_WindowRegistry;

}  // namespace

//----------------------------------------------------------------------------
//  GLFW Window Implementation
//----------------------------------------------------------------------------

namespace {

void setupGlfwWindowHints(const WindowDesc& desc)
{
    glfwDefaultWindowHints();
    // Common Window hints
    glfwWindowHint(GLFW_RESIZABLE, desc.resizable);
    glfwWindowHint(GLFW_VISIBLE, desc.visible);
    glfwWindowHint(GLFW_FOCUSED, desc.focused);
    glfwWindowHint(GLFW_DECORATED, desc.decorated);
    // special Window Hints
    const auto visitor = Visitor{
        [](NoneGraphicsModeTag mode) {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        },
        [](OpenGLGraphicsModeTag mode) {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, mode.config.major);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, mode.config.minor);
            glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, mode.config.coreProfile);
        },
    };
    std::visit(visitor, desc.mode);
}

void registerGlfwCallbacks(GLFWwindow* const handle)
{
    glfwSetFramebufferSizeCallback(handle, [](GLFWwindow* win, int w, int h) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(win));
        uint32_t width = static_cast<uint32_t>(w);
        uint32_t height = static_cast<uint32_t>(h);
        if (width == 0 || height == 0) {
            return;
        }
        self->handleEvent(
            Event::FrameBufferResized{
                .width = width,
                .height = height,
            });
    });

    glfwSetWindowCloseCallback(handle, [](GLFWwindow* window) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        self->handleEvent(Event::Closed{});
        glfwSetWindowShouldClose(window, GLFW_FALSE);
    });

    glfwSetWindowFocusCallback(handle, [](GLFWwindow* window, int focused) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        if (focused == GLFW_TRUE) {
            self->handleEvent(Event::FocusGained{});
        } else {
            self->handleEvent(Event::FocusLost{});
        }
    });

    glfwSetWindowSizeCallback(handle, [](GLFWwindow* window, int width, int height) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        self->handleEvent(
            Event::Resized{
                .width = width,
                .height = height,
            });
    });

    glfwSetKeyCallback(handle, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        Key mappedKey = inputmap::toKey(key);
        if (mappedKey == Key::Unknown) {
            return;
        }
        if (action == GLFW_PRESS) {
            self->handleEvent(
                Event::KeyPressed{
                    .key = mappedKey,
                    .scancode = scancode,
                    .alt = (mods & GLFW_MOD_ALT) != 0,
                    .control = (mods & GLFW_MOD_CONTROL) != 0,
                    .shift = (mods & GLFW_MOD_SHIFT) != 0,
                    .system = (mods & GLFW_MOD_SUPER) != 0,
                });
        } else if (action == GLFW_RELEASE) {
            self->handleEvent(
                Event::KeyReleased{
                    .key = mappedKey,
                    .scancode = scancode,
                    .alt = (mods & GLFW_MOD_ALT) != 0,
                    .control = (mods & GLFW_MOD_CONTROL) != 0,
                    .shift = (mods & GLFW_MOD_SHIFT) != 0,
                    .system = (mods & GLFW_MOD_SUPER) != 0,
                });
        }
    });

    glfwSetMouseButtonCallback(handle, [](GLFWwindow* window, int button, int action, int mods) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        MouseButton mappedButton = inputmap::toMouseButton(button);
        if (mappedButton == MouseButton::Unknown) {
            return;
        }
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        if (action == GLFW_PRESS) {
            self->handleEvent(
                Event::MouseButtonPressed{
                    .button = mappedButton,
                    .posX = xpos,
                    .posY = ypos,
                    .alt = (mods & GLFW_MOD_ALT) != 0,
                    .control = (mods & GLFW_MOD_CONTROL) != 0,
                    .shift = (mods & GLFW_MOD_SHIFT) != 0,
                    .system = (mods & GLFW_MOD_SUPER) != 0,
                });
        } else if (action == GLFW_RELEASE) {
            self->handleEvent(
                Event::MouseButtonReleased{
                    .button = mappedButton,
                    .posX = xpos,
                    .posY = ypos,
                    .alt = (mods & GLFW_MOD_ALT) != 0,
                    .control = (mods & GLFW_MOD_CONTROL) != 0,
                    .shift = (mods & GLFW_MOD_SHIFT) != 0,
                    .system = (mods & GLFW_MOD_SUPER) != 0,
                });
        }
    });

    glfwSetScrollCallback(handle, [](GLFWwindow* window, double xoffset, double yoffset) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        self->handleEvent(
            Event::MouseWheelScrolled{
                .deltaX = xoffset,
                .deltaY = yoffset,
                .posX = xpos,
                .posY = ypos,
            });
    });

    glfwSetCursorPosCallback(handle, [](GLFWwindow* window, double xpos, double ypos) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        self->handleEvent(
            Event::MouseMoved{
                .posX = xpos,
                .posY = ypos,
            });
    });
}

}  // namespace

GLFWNativeWindow::GLFWNativeWindow(WindowDesc desc)
{
    setupGlfwWindowHints(desc);
    handle_.reset(glfwCreateWindow(
        desc.width,
        desc.height,
        desc.title.c_str(),
        nullptr,  // monitor (fullscreen later)
        nullptr   // share
        ));

    if (!handle_) {
        throw GLFWException("Failed to create window");
    }

    // issue: https://github.com/glfw/glfw/issues/2060
    if (!desc.decorated) {
        glfwSetWindowAttrib(handle_.get(), GLFW_DECORATED, GLFW_FALSE);
    };

    // set data and register callbacks
    glfwSetWindowUserPointer(handle_.get(), this);
    registerGlfwCallbacks(handle_.get());
    // create storage and register to registry
    storage_ = std::make_shared<WindowStorage>();

    // register to registry
    g_WindowRegistry.registerStorage(storage_);
}

void GLFWNativeWindow::handleEvent(Event&& event)
{
    storage_->eventQueue.push_back(std::move(event));
}

NativeHandles GLFWNativeWindow::getNativeHandles() const
{
    NativeHandles handles{};

#if defined(CPPWINDOW_PLATFORM_WINDOWS)
    handles.system = NativeHandles::System::Win32;
    handles.window = glfwGetWin32Window(handle_.get());
#elif defined(CPPWINDOW_PLATFORM_MACOS)
    handles.system = NativeHandles::System::Cocoa;
    handles.window = glfwGetCocoaWindow(handle_.get());
#elif defined(CPPWINDOW_PLATFORM_LINUX)
    int platform = glfwGetPlatform();
    if (platform == GLFW_PLATFORM_X11) {
        handles.system = NativeHandles::System::X11;
        handles.window = reinterpret_cast<void*>(glfwGetX11Window(m_window));
        handles.display = glfwGetX11Display();
    } else if (platform == GLFW_PLATFORM_WAYLAND) {
        handles.system = NativeHandles::System::Wayland;
        handles.window = glfwGetWaylandWindow(m_window);
        handles.display = glfwGetWaylandDisplay();
    }
#endif

    return handles;
}

VulkanHandle GLFWNativeWindow::createVulkanSurface(void* instance) const
{
    VkSurfaceKHR surface = 0;

    // 3. Call the function directly
    // The linker will find this inside the glfw3 library you are linking against
    VkResult result = glfwCreateWindowSurface(
        static_cast<VkInstance>(instance),
        handle_.get(),
        nullptr,
        &surface);
    // VK_SUCCESS is 0
    if (result != 0) {
        throw GLFWException("Failed to create window surface via GLFW");
    }
    return static_cast<VulkanHandle>(surface);
}

void GLFWNativeWindow::makeContextCurrent()
{
    glfwMakeContextCurrent(handle_.get());
}

void GLFWNativeWindow::swapBuffers()
{
    glfwSwapBuffers(handle_.get());
}

bool GLFWNativeWindow::shouldClose() const noexcept
{
    return glfwWindowShouldClose(handle_.get()) == GLFW_TRUE;
}

void GLFWNativeWindow::requestClose() noexcept
{
    glfwSetWindowShouldClose(handle_.get(), GLFW_TRUE);
}

std::span<Event> GLFWNativeWindow::events() const noexcept
{
    return storage_->eventQueue;
}

const NativeInputState* GLFWNativeWindow::getInput() const noexcept
{
    return storage_->inputState.get();
}

void GLFWNativeWindow::setTitle(const std::string& title)
{
    glfwSetWindowTitle(handle_.get(), title.c_str());
}

void GLFWNativeWindow::setSize(int width, int height)
{
    glfwSetWindowSize(handle_.get(), width, height);
}

void GLFWNativeWindow::setFocus(bool focus) const noexcept
{
    glfwSetWindowAttrib(handle_.get(), GLFW_FOCUSED, focus);
}

void GLFWNativeWindow::setVisible(bool visible) const noexcept
{
    glfwSetWindowAttrib(handle_.get(), GLFW_VISIBLE, visible);
}

std::pair<int, int> GLFWNativeWindow::getSize() const noexcept
{
    int width, height;
    glfwGetWindowSize(handle_.get(), &width, &height);
    return {
        width,
        height,
    };
}

std::pair<uint32_t, uint32_t> GLFWNativeWindow::getFrameBufferSize() const noexcept
{
    int width, height;
    glfwGetFramebufferSize(handle_.get(), &width, &height);
    return {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height),
    };
}

bool GLFWNativeWindow::isFocused() const noexcept
{
    return glfwGetWindowAttrib(handle_.get(), GLFW_FOCUSED) == GLFW_TRUE;
}

bool GLFWNativeWindow::isVisible() const noexcept
{
    return glfwGetWindowAttrib(handle_.get(), GLFW_VISIBLE) == GLFW_TRUE;
}

//----------------------------------------------------------------------------
//  GLFW Context Implementation
//----------------------------------------------------------------------------
GLFWWindowContext::GLFWWindowContext()
{
    if (!glfwInit()) {
        throw GLFWException("Failed to initialize GLFW");
    }
}

GLFWWindowContext::~GLFWWindowContext()
{
    glfwTerminate();
}

void GLFWWindowContext::pollEvents() noexcept
{
    // clear old event buffers
    g_WindowRegistry.resetAll();
    // poll new events
    glfwPollEvents();
}

ProcLoader GLFWWindowContext::getProcLoader() const
{
    return [](const char* name) -> ProcFunction {
        return glfwGetProcAddress(name);
    };
}

bool GLFWWindowContext::isVulkanSupported() const
{
    return glfwVulkanSupported();
}

std::vector<std::string> GLFWWindowContext::getRequiredVulkanExtensions() const
{
    uint32_t count = 0;
    const char** ext = glfwGetRequiredInstanceExtensions(&count);

    return std::vector<std::string>(ext, ext + count);
}

//----------------------------------------------------------------------------
//  GLFW Factory Implementation
//----------------------------------------------------------------------------
namespace factory {

std::unique_ptr<NativeWindowContext> createNativeContext()
{
    return std::make_unique<GLFWWindowContext>();
}

std::unique_ptr<NativeWindow> createNativeWindow(WindowDesc desc)
{
    return std::make_unique<GLFWNativeWindow>(std::move(desc));
}

}  // namespace factory
}  // namespace cwin