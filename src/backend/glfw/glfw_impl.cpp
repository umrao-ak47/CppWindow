/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#include "glfw_impl.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

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
//  GLFW Error Handling
//----------------------------------------------------------------------------
namespace {

struct GlfwError
{
    int code = GLFW_NO_ERROR;
    std::string description;
};

thread_local GlfwError g_LastGlfwError;

void glfwErrorCallback(int code, const char* description)
{
    g_LastGlfwError.code = code;
    g_LastGlfwError.description = description ? description : "";
}

GlfwError takeGlfwError()
{
    const char* description = nullptr;
    const int code = glfwGetError(&description);
    if (code != GLFW_NO_ERROR) {
        g_LastGlfwError = {};
        return GlfwError{
            .code = code,
            .description = description ? description : "",
        };
    }

    GlfwError stored = std::move(g_LastGlfwError);
    g_LastGlfwError = {};
    return stored;
}

void clearGlfwError()
{
    (void)takeGlfwError();
}

Error makeGlfwError(ErrorCode code, const std::string& message)
{
    GlfwError glfwError = takeGlfwError();
    std::string fullMessage = message;

    if (glfwError.code != GLFW_NO_ERROR) {
        fullMessage += " [GLFW code " + std::to_string(glfwError.code);
        if (!glfwError.description.empty()) {
            fullMessage += ": " + glfwError.description;
        }
        fullMessage += "]";
    }

    return Error(code, std::move(fullMessage));
}

Error makeGlfwError(ErrorCode code, const std::string& message, const std::string& detail)
{
    Error error = makeGlfwError(code, message);
    std::string fullMessage = error.what();
    if (!detail.empty()) {
        fullMessage += " [" + detail + "]";
    }
    return Error(code, std::move(fullMessage));
}

//----------------------------------------------------------------------------
//  GLFW Input Mapping Implementation
//----------------------------------------------------------------------------
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

namespace {

[[nodiscard]] bool isValidKey(Key key) noexcept
{
    const auto idx = static_cast<size_t>(key);
    return idx > static_cast<size_t>(Key::Unknown) && idx < KeyCount;
}

[[nodiscard]] bool isValidMouseButton(MouseButton button) noexcept
{
    const auto idx = static_cast<size_t>(button);
    return idx > static_cast<size_t>(MouseButton::Unknown) && idx < MouseButtonCount;
}

}  // namespace

GLFWInputState::GLFWInputState(GLFWwindow* window)
    : window_(window)
{
    if (!window) {
        return;
    }

    glfwGetCursorPos(window, &mousePosX_, &mousePosY_);
    hasMousePosition_ = true;
    mouseInside_ = glfwGetWindowAttrib(window, GLFW_HOVERED) == GLFW_TRUE;
}

bool GLFWInputState::queryMouseInside() const
{
    if (!window_) {
        return mouseInside_;
    }

    if (glfwGetInputMode(window_, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
        return glfwGetWindowAttrib(window_, GLFW_FOCUSED) == GLFW_TRUE;
    }

    return glfwGetWindowAttrib(window_, GLFW_HOVERED) == GLFW_TRUE;
}

void GLFWInputState::handleEvent(const Event& event)
{
    event.visit([&](auto&& event) {
        using T = std::decay_t<decltype(event)>;
        if constexpr (std::is_same_v<T, Event::KeyPressed>) {
            if (isValidKey(event.key)) {
                keyStates_.set(static_cast<size_t>(event.key));
            }
        } else if constexpr (std::is_same_v<T, Event::KeyReleased>) {
            if (isValidKey(event.key)) {
                keyStates_.reset(static_cast<size_t>(event.key));
            }
        } else if constexpr (std::is_same_v<T, Event::MouseButtonPressed>) {
            if (isValidMouseButton(event.button)) {
                mouseStates_.set(static_cast<size_t>(event.button));
            }
        } else if constexpr (std::is_same_v<T, Event::MouseButtonReleased>) {
            if (isValidMouseButton(event.button)) {
                mouseStates_.reset(static_cast<size_t>(event.button));
            }
        } else if constexpr (std::is_same_v<T, Event::MouseWheelScrolled>) {
            scrollDeltaX_ += event.deltaX;
            scrollDeltaY_ += event.deltaY;
        } else if constexpr (std::is_same_v<T, Event::MouseMoved>) {
            if (hasMousePosition_) {
                mouseDeltaX_ += event.posX - mousePosX_;
                mouseDeltaY_ += event.posY - mousePosY_;
            }
            mousePosX_ = event.posX;
            mousePosY_ = event.posY;
            hasMousePosition_ = true;
        } else if constexpr (std::is_same_v<T, Event::MouseEntered>) {
            mouseInside_ = true;
        } else if constexpr (std::is_same_v<T, Event::MouseLeft>) {
            mouseInside_ = false;
        } else if constexpr (std::is_same_v<T, Event::FocusLost>) {
            keyStates_.reset();
            mouseStates_.reset();
        }
    });
}

bool GLFWInputState::isKeyDown(Key key) const
{
    if (!isValidKey(key)) {
        return false;
    }

    return keyStates_.test(static_cast<size_t>(key));
}

bool GLFWInputState::isKeyPressed(Key key) const
{
    if (!isValidKey(key)) {
        return false;
    }

    size_t idx = static_cast<size_t>(key);
    return keyStates_.test(idx) && !prevKeyStates_.test(idx);
}

bool GLFWInputState::isKeyReleased(Key key) const
{
    if (!isValidKey(key)) {
        return false;
    }

    size_t idx = static_cast<size_t>(key);
    return !keyStates_.test(idx) && prevKeyStates_.test(idx);
}

bool GLFWInputState::isMouseButtonDown(MouseButton button) const
{
    if (!isValidMouseButton(button)) {
        return false;
    }

    return mouseStates_.test(static_cast<size_t>(button));
}

bool GLFWInputState::isMouseButtonPressed(MouseButton button) const
{
    if (!isValidMouseButton(button)) {
        return false;
    }

    size_t idx = static_cast<size_t>(button);
    return mouseStates_.test(idx) && !prevMouseStates_.test(idx);
}

bool GLFWInputState::isMouseButtonReleased(MouseButton button) const
{
    if (!isValidMouseButton(button)) {
        return false;
    }

    size_t idx = static_cast<size_t>(button);
    return !mouseStates_.test(idx) && prevMouseStates_.test(idx);
}

std::pair<double, double> GLFWInputState::getMousePosition() const
{
    return { mousePosX_, mousePosY_ };
}

void GLFWInputState::setMousePosition(double x, double y)
{
    mousePosX_ = x;
    mousePosY_ = y;
    hasMousePosition_ = true;
}

std::pair<double, double> GLFWInputState::getMouseDelta() const
{
    return { mouseDeltaX_, mouseDeltaY_ };
}

std::pair<double, double> GLFWInputState::getScrollDelta() const
{
    return { scrollDeltaX_, scrollDeltaY_ };
}

bool GLFWInputState::isMouseInside() const
{
    return queryMouseInside();
}

void GLFWInputState::reset()
{
    prevKeyStates_ = keyStates_;
    prevMouseStates_ = mouseStates_;
    mouseDeltaX_ = 0;
    mouseDeltaY_ = 0;
    scrollDeltaX_ = 0;
    scrollDeltaY_ = 0;
    mouseInside_ = queryMouseInside();
}

//----------------------------------------------------------------------------
//  GLFW Window Registry Implementation
//----------------------------------------------------------------------------
namespace {

// store all refs to windows storage so we can clean it
// per frame
WindowStorageRegistry<WindowStorage> g_WindowRegistry;

std::vector<GLFWmonitor*> getOrderedMonitors()
{
    int count = 0;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    std::vector<GLFWmonitor*> ordered;
    if (!monitors || count <= 0) {
        return ordered;
    }

    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    if (primary) {
        ordered.push_back(primary);
    }

    for (int i = 0; i < count; ++i) {
        if (monitors[i] && monitors[i] != primary) {
            ordered.push_back(monitors[i]);
        }
    }

    return ordered;
}

GLFWmonitor* getMonitorById(uint32_t monitorId)
{
    auto monitors = getOrderedMonitors();
    if (monitorId >= monitors.size()) {
        return nullptr;
    }
    return monitors[monitorId];
}

VideoMode toVideoMode(const GLFWvidmode& mode)
{
    return {
        .width = mode.width,
        .height = mode.height,
        .redBits = mode.redBits,
        .greenBits = mode.greenBits,
        .blueBits = mode.blueBits,
        .refreshRate = mode.refreshRate,
    };
}

MonitorInfo toMonitorInfo(GLFWmonitor* monitor, uint32_t id, bool primary)
{
    MonitorInfo info{};
    info.id = id;
    info.primary = primary;

    if (const char* name = glfwGetMonitorName(monitor)) {
        info.name = name;
    }

    glfwGetMonitorPos(monitor, &info.x, &info.y);
    glfwGetMonitorPhysicalSize(monitor, &info.physicalWidthMM, &info.physicalHeightMM);
    glfwGetMonitorContentScale(monitor, &info.contentScaleX, &info.contentScaleY);

    if (const GLFWvidmode* mode = glfwGetVideoMode(monitor)) {
        info.currentVideoMode = toVideoMode(*mode);
    }

    return info;
}

int toGlfwCursorMode(CursorMode mode)
{
    switch (mode) {
        case CursorMode::Normal:
            return GLFW_CURSOR_NORMAL;
        case CursorMode::Hidden:
            return GLFW_CURSOR_HIDDEN;
        case CursorMode::Captured:
            return GLFW_CURSOR_DISABLED;
    }

    return GLFW_CURSOR_NORMAL;
}

int toGlfwCursorShape(CursorShape shape)
{
    switch (shape) {
        case CursorShape::Arrow:
            return GLFW_ARROW_CURSOR;
        case CursorShape::IBeam:
            return GLFW_IBEAM_CURSOR;
        case CursorShape::Crosshair:
            return GLFW_CROSSHAIR_CURSOR;
        case CursorShape::Hand:
            return GLFW_POINTING_HAND_CURSOR;
        case CursorShape::ResizeHorizontal:
            return GLFW_RESIZE_EW_CURSOR;
        case CursorShape::ResizeVertical:
            return GLFW_RESIZE_NS_CURSOR;
        case CursorShape::ResizeDiagonalNWSE:
            return GLFW_RESIZE_NWSE_CURSOR;
        case CursorShape::ResizeDiagonalNESW:
            return GLFW_RESIZE_NESW_CURSOR;
        case CursorShape::ResizeAll:
            return GLFW_RESIZE_ALL_CURSOR;
        case CursorShape::NotAllowed:
            return GLFW_NOT_ALLOWED_CURSOR;
    }

    return GLFW_ARROW_CURSOR;
}

bool isValidImage(const ImageRgba& image) noexcept
{
    if (image.width == 0 || image.height == 0) {
        return false;
    }

    const size_t maxSize = std::numeric_limits<size_t>::max();
    if (static_cast<size_t>(image.width) > maxSize / static_cast<size_t>(image.height)) {
        return false;
    }

    const size_t pixelCount = static_cast<size_t>(image.width) * static_cast<size_t>(image.height);
    if (pixelCount > maxSize / 4) {
        return false;
    }

    return image.pixels.size() >= pixelCount * 4;
}

std::optional<GLFWimage> toGlfwImage(const ImageRgba& image) noexcept
{
    if (!isValidImage(image)) {
        return std::nullopt;
    }

    if (image.width > static_cast<uint32_t>(std::numeric_limits<int>::max()) ||
        image.height > static_cast<uint32_t>(std::numeric_limits<int>::max())) {
        return std::nullopt;
    }

    return GLFWimage{
        .width = static_cast<int>(image.width),
        .height = static_cast<int>(image.height),
        .pixels = const_cast<unsigned char*>(image.pixels.data()),
    };
}

Modifiers toModifiers(int mods)
{
    return Modifiers{
        .alt = (mods & GLFW_MOD_ALT) != 0,
        .control = (mods & GLFW_MOD_CONTROL) != 0,
        .shift = (mods & GLFW_MOD_SHIFT) != 0,
        .system = (mods & GLFW_MOD_SUPER) != 0,
    };
}

uint32_t getWindowMonitorId(GLFWwindow* window)
{
    auto monitors = getOrderedMonitors();
    if (monitors.empty()) {
        return 0;
    }

    if (GLFWmonitor* fullscreenMonitor = glfwGetWindowMonitor(window)) {
        for (size_t i = 0; i < monitors.size(); ++i) {
            if (monitors[i] == fullscreenMonitor) {
                return static_cast<uint32_t>(i);
            }
        }
    }

    int windowX = 0;
    int windowY = 0;
    int windowWidth = 0;
    int windowHeight = 0;
    glfwGetWindowPos(window, &windowX, &windowY);
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    int bestArea = -1;
    uint32_t bestId = 0;

    for (size_t i = 0; i < monitors.size(); ++i) {
        int monitorX = 0;
        int monitorY = 0;
        glfwGetMonitorPos(monitors[i], &monitorX, &monitorY);

        const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
        if (!mode) {
            continue;
        }

        const int overlapLeft = std::max(windowX, monitorX);
        const int overlapTop = std::max(windowY, monitorY);
        const int overlapRight = std::min(windowX + windowWidth, monitorX + mode->width);
        const int overlapBottom = std::min(windowY + windowHeight, monitorY + mode->height);
        const int overlapWidth = std::max(0, overlapRight - overlapLeft);
        const int overlapHeight = std::max(0, overlapBottom - overlapTop);
        const int overlapArea = overlapWidth * overlapHeight;

        if (overlapArea > bestArea) {
            bestArea = overlapArea;
            bestId = static_cast<uint32_t>(i);
        }
    }

    return bestId;
}

int toGlfwGamepadButton(GamepadButton button)
{
    switch (button) {
        case GamepadButton::A:
            return GLFW_GAMEPAD_BUTTON_A;
        case GamepadButton::B:
            return GLFW_GAMEPAD_BUTTON_B;
        case GamepadButton::X:
            return GLFW_GAMEPAD_BUTTON_X;
        case GamepadButton::Y:
            return GLFW_GAMEPAD_BUTTON_Y;
        case GamepadButton::LeftBumper:
            return GLFW_GAMEPAD_BUTTON_LEFT_BUMPER;
        case GamepadButton::RightBumper:
            return GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER;
        case GamepadButton::Back:
            return GLFW_GAMEPAD_BUTTON_BACK;
        case GamepadButton::Start:
            return GLFW_GAMEPAD_BUTTON_START;
        case GamepadButton::Guide:
            return GLFW_GAMEPAD_BUTTON_GUIDE;
        case GamepadButton::LeftThumb:
            return GLFW_GAMEPAD_BUTTON_LEFT_THUMB;
        case GamepadButton::RightThumb:
            return GLFW_GAMEPAD_BUTTON_RIGHT_THUMB;
        case GamepadButton::DPadUp:
            return GLFW_GAMEPAD_BUTTON_DPAD_UP;
        case GamepadButton::DPadRight:
            return GLFW_GAMEPAD_BUTTON_DPAD_RIGHT;
        case GamepadButton::DPadDown:
            return GLFW_GAMEPAD_BUTTON_DPAD_DOWN;
        case GamepadButton::DPadLeft:
            return GLFW_GAMEPAD_BUTTON_DPAD_LEFT;
    }

    return GLFW_GAMEPAD_BUTTON_A;
}

int toGlfwGamepadAxis(GamepadAxis axis)
{
    switch (axis) {
        case GamepadAxis::LeftX:
            return GLFW_GAMEPAD_AXIS_LEFT_X;
        case GamepadAxis::LeftY:
            return GLFW_GAMEPAD_AXIS_LEFT_Y;
        case GamepadAxis::RightX:
            return GLFW_GAMEPAD_AXIS_RIGHT_X;
        case GamepadAxis::RightY:
            return GLFW_GAMEPAD_AXIS_RIGHT_Y;
        case GamepadAxis::LeftTrigger:
            return GLFW_GAMEPAD_AXIS_LEFT_TRIGGER;
        case GamepadAxis::RightTrigger:
            return GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER;
    }

    return GLFW_GAMEPAD_AXIS_LEFT_X;
}

GamepadButton toGamepadButton(size_t index)
{
    return static_cast<GamepadButton>(index);
}

GamepadAxis toGamepadAxis(size_t index)
{
    return static_cast<GamepadAxis>(index);
}

std::optional<GamepadState> readStandardGamepadState(uint32_t gamepadId)
{
    if (gamepadId >= MaxGamepads || !glfwJoystickPresent(static_cast<int>(gamepadId))) {
        return std::nullopt;
    }

    const int joystickId = static_cast<int>(gamepadId);
    if (glfwJoystickIsGamepad(joystickId) != GLFW_TRUE) {
        return std::nullopt;
    }

    GLFWgamepadstate glfwState{};
    if (glfwGetGamepadState(joystickId, &glfwState) != GLFW_TRUE) {
        return std::nullopt;
    }

    GamepadState state{};
    state.id = gamepadId;
    state.standardMapping = true;

    if (const char* name = glfwGetGamepadName(joystickId)) {
        state.name = name;
    } else if (const char* name = glfwGetJoystickName(joystickId)) {
        state.name = name;
    }

    for (size_t i = 0; i < GamepadButtonCount; ++i) {
        state.buttons[i] =
            glfwState.buttons[toGlfwGamepadButton(toGamepadButton(i))] == GLFW_PRESS;
    }

    for (size_t i = 0; i < GamepadAxisCount; ++i) {
        state.axes[i] = glfwState.axes[toGlfwGamepadAxis(toGamepadAxis(i))];
    }

    return state;
}

std::optional<GamepadInfo> readGamepadInfo(uint32_t gamepadId)
{
    if (gamepadId >= MaxGamepads || !glfwJoystickPresent(static_cast<int>(gamepadId))) {
        return std::nullopt;
    }

    const int joystickId = static_cast<int>(gamepadId);
    GamepadInfo info{};
    info.id = gamepadId;
    info.standardMapping = glfwJoystickIsGamepad(joystickId) == GLFW_TRUE;

    const char* name =
        info.standardMapping ? glfwGetGamepadName(joystickId) : glfwGetJoystickName(joystickId);
    if (!name) {
        name = glfwGetJoystickName(joystickId);
    }
    if (name) {
        info.name = name;
    }

    return info;
}

struct JoystickSnapshot
{
    std::string name;
    bool standardMapping = false;
    std::vector<unsigned char> buttons;
    std::vector<float> axes;
};

std::optional<JoystickSnapshot> readJoystickSnapshot(uint32_t joystickId)
{
    if (joystickId >= MaxJoysticks || !glfwJoystickPresent(static_cast<int>(joystickId))) {
        return std::nullopt;
    }

    const int backendId = static_cast<int>(joystickId);
    JoystickSnapshot snapshot{};
    snapshot.standardMapping = glfwJoystickIsGamepad(backendId) == GLFW_TRUE;

    if (const char* name = glfwGetJoystickName(backendId)) {
        snapshot.name = name;
    }

    int buttonCount = 0;
    if (const unsigned char* buttons = glfwGetJoystickButtons(backendId, &buttonCount)) {
        snapshot.buttons.assign(buttons, buttons + buttonCount);
    }

    int axisCount = 0;
    if (const float* axes = glfwGetJoystickAxes(backendId, &axisCount)) {
        snapshot.axes.assign(axes, axes + axisCount);
    }

    return snapshot;
}

void dispatchEventToAllWindows(const Event& event)
{
    g_WindowRegistry.forEach([&](WindowStorage& storage) {
        storage.inputState->handleEvent(event);
        storage.eventQueue.push_back(event);
    });
}

void pollJoysticks()
{
    static std::array<std::optional<JoystickSnapshot>, MaxJoysticks> previousStates{};
    constexpr float AxisEpsilon = 0.01f;

    for (uint32_t id = 0; id < MaxJoysticks; ++id) {
        auto current = readJoystickSnapshot(id);
        auto& previous = previousStates[id];

        if (current && !previous) {
            dispatchEventToAllWindows(
                Event::JoystickConnected{
                    .joystickId = id,
                    .name = current->name,
                    .standardMapping = current->standardMapping,
                    .axisCount = static_cast<uint32_t>(current->axes.size()),
                    .buttonCount = static_cast<uint32_t>(current->buttons.size()),
                });
        } else if (!current && previous) {
            dispatchEventToAllWindows(
                Event::JoystickDisconnected{
                    .joystickId = id,
                });
        } else if (current && previous) {
            const size_t buttonCount = std::max(current->buttons.size(), previous->buttons.size());
            for (size_t button = 0; button < buttonCount; ++button) {
                const bool currentDown =
                    button < current->buttons.size() && current->buttons[button] == GLFW_PRESS;
                const bool previousDown =
                    button < previous->buttons.size() && previous->buttons[button] == GLFW_PRESS;
                if (currentDown == previousDown) {
                    continue;
                }

                if (currentDown) {
                    dispatchEventToAllWindows(
                        Event::JoystickButtonPressed{
                            .joystickId = id,
                            .button = static_cast<uint32_t>(button),
                        });
                } else {
                    dispatchEventToAllWindows(
                        Event::JoystickButtonReleased{
                            .joystickId = id,
                            .button = static_cast<uint32_t>(button),
                        });
                }
            }

            const size_t axisCount = std::max(current->axes.size(), previous->axes.size());
            for (size_t axis = 0; axis < axisCount; ++axis) {
                const float currentValue = axis < current->axes.size() ? current->axes[axis] : 0.0f;
                const float previousValue =
                    axis < previous->axes.size() ? previous->axes[axis] : 0.0f;
                if (std::abs(currentValue - previousValue) <= AxisEpsilon) {
                    continue;
                }

                dispatchEventToAllWindows(
                    Event::JoystickMoved{
                        .joystickId = id,
                        .axis = static_cast<uint32_t>(axis),
                        .position = currentValue,
                    });
            }
        }

        previous = std::move(current);
    }
}

void pollGamepads()
{
    static std::array<std::optional<GamepadState>, MaxGamepads> previousStates{};
    constexpr float AxisEpsilon = 0.01f;

    for (uint32_t id = 0; id < MaxGamepads; ++id) {
        auto current = readStandardGamepadState(id);
        auto& previous = previousStates[id];

        if (current && !previous) {
            dispatchEventToAllWindows(
                Event::GamepadConnected{
                    .gamepadId = id,
                    .name = current->name,
                    .standardMapping = current->standardMapping,
                });
        } else if (!current && previous) {
            dispatchEventToAllWindows(
                Event::GamepadDisconnected{
                    .gamepadId = id,
                });
        } else if (current && previous) {
            for (size_t button = 0; button < GamepadButtonCount; ++button) {
                if (current->buttons[button] == previous->buttons[button]) {
                    continue;
                }

                if (current->buttons[button]) {
                    dispatchEventToAllWindows(
                        Event::GamepadButtonPressed{
                            .gamepadId = id,
                            .button = toGamepadButton(button),
                        });
                } else {
                    dispatchEventToAllWindows(
                        Event::GamepadButtonReleased{
                            .gamepadId = id,
                            .button = toGamepadButton(button),
                        });
                }
            }

            for (size_t axis = 0; axis < GamepadAxisCount; ++axis) {
                const float currentValue = current->axes[axis];
                const float previousValue = previous->axes[axis];
                if (std::abs(currentValue - previousValue) <= AxisEpsilon) {
                    continue;
                }

                dispatchEventToAllWindows(
                    Event::GamepadAxisMoved{
                        .gamepadId = id,
                        .axis = toGamepadAxis(axis),
                        .value = currentValue,
                    });
            }
        }

        previous = std::move(current);
    }
}

}  // namespace

//----------------------------------------------------------------------------
//  GLFW Window Implementation
//----------------------------------------------------------------------------

namespace {

void setupGlfwWindowHints(const WindowDesc& desc)
{
    glfwDefaultWindowHints();
    const bool createVisible =
        desc.visible && !desc.position && !desc.opacity && desc.windowMode == WindowMode::Windowed;

    // Common Window hints
    glfwWindowHint(GLFW_RESIZABLE, desc.resizable);
    glfwWindowHint(GLFW_VISIBLE, createVisible ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_FOCUSED, desc.focused);
    glfwWindowHint(GLFW_DECORATED, desc.decorated);
    glfwWindowHint(GLFW_FLOATING, desc.floating);
    // special Window Hints
    const auto visitor = Visitor{
        [](NoneGraphicsModeTag) {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        },
        [](OpenGLGraphicsModeTag mode) {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, mode.config.major);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, mode.config.minor);
            glfwWindowHint(
                GLFW_OPENGL_PROFILE,
                mode.config.coreProfile ? GLFW_OPENGL_CORE_PROFILE : GLFW_OPENGL_ANY_PROFILE);
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
        self->handleMonitorChanged(getWindowMonitorId(window));
        self->handleEvent(
            Event::Resized{
                .width = width,
                .height = height,
            });
    });

    glfwSetWindowPosCallback(handle, [](GLFWwindow* window, int x, int y) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        self->handleMonitorChanged(getWindowMonitorId(window));
        self->handleEvent(
            Event::Moved{
                .x = x,
                .y = y,
            });
    });

    glfwSetWindowIconifyCallback(handle, [](GLFWwindow* window, int iconified) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        if (iconified == GLFW_TRUE) {
            self->handleEvent(Event::Minimized{});
        } else {
            self->handleEvent(Event::Restored{});
        }
    });

    glfwSetWindowMaximizeCallback(handle, [](GLFWwindow* window, int maximized) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        if (maximized == GLFW_TRUE) {
            self->handleEvent(Event::Maximized{});
        } else {
            self->handleEvent(Event::Restored{});
        }
    });

    glfwSetWindowContentScaleCallback(handle, [](GLFWwindow* window, float xScale, float yScale) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        self->handleEvent(
            Event::ContentScaleChanged{
                .xScale = xScale,
                .yScale = yScale,
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
                    .modifiers = toModifiers(mods),
                });
        } else if (action == GLFW_RELEASE) {
            self->handleEvent(
                Event::KeyReleased{
                    .key = mappedKey,
                    .scancode = scancode,
                    .modifiers = toModifiers(mods),
                });
        }
    });

    glfwSetCharCallback(handle, [](GLFWwindow* window, unsigned int codepoint) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        self->handleEvent(
            Event::TextEntered{
                .unicode = static_cast<char32_t>(codepoint),
            });
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
                    .modifiers = toModifiers(mods),
                });
        } else if (action == GLFW_RELEASE) {
            self->handleEvent(
                Event::MouseButtonReleased{
                    .button = mappedButton,
                    .posX = xpos,
                    .posY = ypos,
                    .modifiers = toModifiers(mods),
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

    glfwSetCursorEnterCallback(handle, [](GLFWwindow* window, int entered) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        if (entered == GLFW_TRUE) {
            self->handleEvent(Event::MouseEntered{});
        } else {
            self->handleEvent(Event::MouseLeft{});
        }
    });

    glfwSetDropCallback(handle, [](GLFWwindow* window, int count, const char** paths) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        std::vector<std::string> droppedPaths;
        droppedPaths.reserve(static_cast<size_t>(std::max(count, 0)));

        for (int i = 0; i < count; ++i) {
            if (paths[i]) {
                droppedPaths.emplace_back(paths[i]);
            }
        }

        double xpos = 0.0;
        double ypos = 0.0;
        glfwGetCursorPos(window, &xpos, &ypos);
        self->handleEvent(
            Event::FilesDropped{
                .paths = std::move(droppedPaths),
                .posX = xpos,
                .posY = ypos,
            });
    });
}

}  // namespace

GLFWNativeWindow::GLFWNativeWindow(WindowDesc desc)
{
    hasOpenGLContext_ = std::holds_alternative<OpenGLGraphicsModeTag>(desc.mode);
    decorated_ = desc.decorated;
    floating_ = desc.floating;
    windowedDecorated_ = decorated_;
    windowedFloating_ = floating_;

    setupGlfwWindowHints(desc);
    clearGlfwError();
    handle_.reset(glfwCreateWindow(
        desc.width,
        desc.height,
        desc.title.c_str(),
        nullptr,  // monitor (fullscreen later)
        nullptr   // share
        ));

    if (!handle_) {
        throw makeGlfwError(ErrorCode::WindowCreationFailed, "Failed to create GLFW window");
    }

    // issue: https://github.com/glfw/glfw/issues/2060
    if (!desc.decorated) {
        glfwSetWindowAttrib(handle_.get(), GLFW_DECORATED, GLFW_FALSE);
    };

    if (desc.position) {
        glfwSetWindowPos(handle_.get(), desc.position->first, desc.position->second);
    }

    storage_ = std::make_shared<WindowStorage>(std::make_unique<GLFWInputState>(handle_.get()));
    captureWindowedBounds();
    currentMonitorId_ = getWindowMonitorId(handle_.get());

    // set data and register callbacks
    glfwSetWindowUserPointer(handle_.get(), this);
    registerGlfwCallbacks(handle_.get());

    if (desc.sizeLimits) {
        setSizeLimits(*desc.sizeLimits);
    }
    if (desc.aspectRatio) {
        setAspectRatio(*desc.aspectRatio);
    }
    if (desc.opacity) {
        setOpacity(*desc.opacity);
    }
    if (desc.cursorMode) {
        setCursorMode(*desc.cursorMode);
    }
    if (desc.rawMouseMotion) {
        setRawMouseMotion(*desc.rawMouseMotion);
    }
    if (desc.vSync) {
        setVSync(*desc.vSync);
    }
    if (desc.windowMode != WindowMode::Windowed) {
        setWindowMode(desc.windowMode, desc.monitorId, desc.videoMode);
    }
    if (desc.visible && (desc.position || desc.opacity || desc.windowMode != WindowMode::Windowed)) {
        glfwShowWindow(handle_.get());
        if (desc.focused) {
            glfwFocusWindow(handle_.get());
        }
    }

    // register to registry
    g_WindowRegistry.registerStorage(storage_);
}

void GLFWNativeWindow::captureWindowedBounds()
{
    if (windowMode_ != WindowMode::Windowed || !handle_) {
        return;
    }

    glfwGetWindowPos(handle_.get(), &windowedX_, &windowedY_);
    glfwGetWindowSize(handle_.get(), &windowedWidth_, &windowedHeight_);
    windowedDecorated_ = decorated_;
    windowedFloating_ = floating_;
}

void GLFWNativeWindow::handleEvent(Event&& event)
{
    storage_->inputState->handleEvent(event);
    storage_->eventQueue.push_back(std::move(event));
}

void GLFWNativeWindow::handleMonitorChanged(uint32_t monitorId)
{
    if (monitorId == currentMonitorId_) {
        return;
    }

    currentMonitorId_ = monitorId;
    handleEvent(
        Event::MonitorChanged{
            .monitorId = currentMonitorId_,
            .mode = windowMode_,
        });
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
        handles.window = reinterpret_cast<void*>(glfwGetX11Window(handle_.get()));
        handles.display = glfwGetX11Display();
    } else if (platform == GLFW_PLATFORM_WAYLAND) {
        handles.system = NativeHandles::System::Wayland;
        handles.window = glfwGetWaylandWindow(handle_.get());
        handles.display = glfwGetWaylandDisplay();
    }
#endif

    return handles;
}

VulkanHandle GLFWNativeWindow::createVulkanSurface(void* instance) const
{
    VkSurfaceKHR surface = 0;

    clearGlfwError();
    // 3. Call the function directly
    // The linker will find this inside the glfw3 library you are linking against
    VkResult result = glfwCreateWindowSurface(
        static_cast<VkInstance>(instance),
        handle_.get(),
        nullptr,
        &surface);
    // VK_SUCCESS is 0
    if (result != 0) {
        throw makeGlfwError(
            ErrorCode::VulkanSurfaceCreationFailed,
            "Failed to create Vulkan window surface",
            "VkResult " + std::to_string(result));
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

std::span<const Event> GLFWNativeWindow::events() const noexcept
{
    return std::span<const Event>{ storage_->eventQueue.data(), storage_->eventQueue.size() };
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
    captureWindowedBounds();
}

void GLFWNativeWindow::setPosition(int x, int y)
{
    glfwSetWindowPos(handle_.get(), x, y);
    captureWindowedBounds();
}

void GLFWNativeWindow::setSizeLimits(const SizeLimits& limits)
{
    glfwSetWindowSizeLimits(
        handle_.get(),
        limits.minWidth.value_or(GLFW_DONT_CARE),
        limits.minHeight.value_or(GLFW_DONT_CARE),
        limits.maxWidth.value_or(GLFW_DONT_CARE),
        limits.maxHeight.value_or(GLFW_DONT_CARE));
}

void GLFWNativeWindow::clearSizeLimits()
{
    glfwSetWindowSizeLimits(
        handle_.get(),
        GLFW_DONT_CARE,
        GLFW_DONT_CARE,
        GLFW_DONT_CARE,
        GLFW_DONT_CARE);
}

void GLFWNativeWindow::setAspectRatio(AspectRatio ratio)
{
    if (ratio.numerator <= 0 || ratio.denominator <= 0) {
        clearAspectRatio();
        return;
    }

    glfwSetWindowAspectRatio(handle_.get(), ratio.numerator, ratio.denominator);
}

void GLFWNativeWindow::clearAspectRatio()
{
    glfwSetWindowAspectRatio(handle_.get(), GLFW_DONT_CARE, GLFW_DONT_CARE);
}

void GLFWNativeWindow::setResizable(bool resizable)
{
    glfwSetWindowAttrib(handle_.get(), GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
}

void GLFWNativeWindow::setDecorated(bool decorated)
{
    glfwSetWindowAttrib(handle_.get(), GLFW_DECORATED, decorated ? GLFW_TRUE : GLFW_FALSE);
    decorated_ = decorated;
    if (windowMode_ == WindowMode::Windowed) {
        windowedDecorated_ = decorated_;
    }
}

void GLFWNativeWindow::setFloating(bool floating)
{
    glfwSetWindowAttrib(handle_.get(), GLFW_FLOATING, floating ? GLFW_TRUE : GLFW_FALSE);
    floating_ = floating;
    if (windowMode_ == WindowMode::Windowed) {
        windowedFloating_ = floating_;
    }
}

void GLFWNativeWindow::setOpacity(float opacity)
{
    glfwSetWindowOpacity(handle_.get(), std::clamp(opacity, 0.0f, 1.0f));
}

void GLFWNativeWindow::setVSync(bool enabled)
{
    if (!hasOpenGLContext_) {
        return;
    }

    GLFWwindow* previous = glfwGetCurrentContext();
    glfwMakeContextCurrent(handle_.get());
    glfwSwapInterval(enabled ? 1 : 0);
    glfwMakeContextCurrent(previous);
}

void GLFWNativeWindow::setCursorMode(CursorMode mode)
{
    glfwSetInputMode(handle_.get(), GLFW_CURSOR, toGlfwCursorMode(mode));
    cursorMode_ = mode;
}

bool GLFWNativeWindow::setCursorShape(CursorShape shape)
{
    UniqueGLFWcursor cursor(glfwCreateStandardCursor(toGlfwCursorShape(shape)));
    if (!cursor) {
        return false;
    }

    glfwSetCursor(handle_.get(), cursor.get());
    cursor_ = std::move(cursor);
    return true;
}

bool GLFWNativeWindow::setCursorImage(const ImageRgba& image, int hotX, int hotY)
{
    std::optional<GLFWimage> glfwImage = toGlfwImage(image);
    if (!glfwImage) {
        return false;
    }

    if (hotX < 0 || hotY < 0 || hotX >= glfwImage->width || hotY >= glfwImage->height) {
        return false;
    }

    UniqueGLFWcursor cursor(glfwCreateCursor(&*glfwImage, hotX, hotY));
    if (!cursor) {
        return false;
    }

    glfwSetCursor(handle_.get(), cursor.get());
    cursor_ = std::move(cursor);
    return true;
}

void GLFWNativeWindow::clearCursor()
{
    glfwSetCursor(handle_.get(), nullptr);
    cursor_.reset();
}

void GLFWNativeWindow::setMousePosition(double x, double y)
{
    glfwSetCursorPos(handle_.get(), x, y);
    storage_->inputState->setMousePosition(x, y);
}

bool GLFWNativeWindow::setRawMouseMotion(bool enabled)
{
    if (enabled && glfwRawMouseMotionSupported() != GLFW_TRUE) {
        return false;
    }

    glfwSetInputMode(handle_.get(), GLFW_RAW_MOUSE_MOTION, enabled ? GLFW_TRUE : GLFW_FALSE);
    return !enabled || isRawMouseMotionEnabled();
}

void GLFWNativeWindow::minimize()
{
    glfwIconifyWindow(handle_.get());
}

void GLFWNativeWindow::maximize()
{
    if (windowMode_ != WindowMode::Windowed) {
        setWindowMode(WindowMode::Windowed, currentMonitorId_, std::nullopt);
    }

    captureWindowedBounds();
    glfwMaximizeWindow(handle_.get());
}

void GLFWNativeWindow::restore()
{
    if (windowMode_ != WindowMode::Windowed) {
        setWindowMode(WindowMode::Windowed, currentMonitorId_, std::nullopt);
    }

    glfwRestoreWindow(handle_.get());
}

void GLFWNativeWindow::setWindowMode(
    WindowMode mode,
    uint32_t monitorId,
    std::optional<VideoMode> videoMode)
{
    if (mode == WindowMode::Windowed) {
        if (windowMode_ == WindowMode::Windowed) {
            return;
        }

        const bool wasExclusiveFullscreen = glfwGetWindowMonitor(handle_.get()) != nullptr;
        windowMode_ = WindowMode::Windowed;
        setDecorated(windowedDecorated_);
        setFloating(windowedFloating_);
        glfwRestoreWindow(handle_.get());

        if (wasExclusiveFullscreen) {
            glfwSetWindowMonitor(
                handle_.get(),
                nullptr,
                windowedX_,
                windowedY_,
                windowedWidth_,
                windowedHeight_,
                GLFW_DONT_CARE);
        } else {
            glfwSetWindowPos(handle_.get(), windowedX_, windowedY_);
            glfwSetWindowSize(handle_.get(), windowedWidth_, windowedHeight_);
        }

        return;
    }

    if (mode == windowMode_) {
        return;
    }

    GLFWmonitor* monitor = getMonitorById(monitorId);
    if (!monitor) {
        monitor = glfwGetPrimaryMonitor();
        monitorId = 0;
    }
    if (!monitor) {
        return;
    }

    if (mode == WindowMode::Fullscreen) {
        captureWindowedBounds();

        if (glfwGetWindowMonitor(handle_.get())) {
            int x = 0;
            int y = 0;
            int width = 0;
            int height = 0;
            glfwGetMonitorWorkarea(monitor, &x, &y, &width, &height);
            if (width <= 0 || height <= 0) {
                const GLFWvidmode* currentMode = glfwGetVideoMode(monitor);
                if (!currentMode) {
                    return;
                }

                glfwGetMonitorPos(monitor, &x, &y);
                width = currentMode->width;
                height = currentMode->height;
            }
            glfwSetWindowMonitor(handle_.get(), nullptr, x, y, width, height, GLFW_DONT_CARE);
        }

        windowMode_ = mode;
        setDecorated(true);
        setFloating(false);
        glfwMaximizeWindow(handle_.get());
        return;
    }

    if (mode == WindowMode::BorderlessFullscreen) {
        captureWindowedBounds();

        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
        glfwGetMonitorWorkarea(monitor, &x, &y, &width, &height);

        if (width <= 0 || height <= 0) {
            const GLFWvidmode* currentMode = glfwGetVideoMode(monitor);
            if (!currentMode) {
                return;
            }

            glfwGetMonitorPos(monitor, &x, &y);
            width = currentMode->width;
            height = currentMode->height;
        }

        windowMode_ = mode;
        setDecorated(false);
        setFloating(true);

        if (glfwGetWindowMonitor(handle_.get())) {
            glfwSetWindowMonitor(handle_.get(), nullptr, x, y, width, height, GLFW_DONT_CARE);
        } else {
            glfwSetWindowPos(handle_.get(), x, y);
            glfwSetWindowSize(handle_.get(), width, height);
        }

        return;
    }

    captureWindowedBounds();

    if (mode != WindowMode::ExclusiveFullscreen) {
        return;
    }

    int x = 0;
    int y = 0;
    glfwGetMonitorPos(monitor, &x, &y);

    VideoMode targetMode{};
    if (videoMode) {
        targetMode = *videoMode;
    } else if (const GLFWvidmode* currentMode = glfwGetVideoMode(monitor)) {
        targetMode = toVideoMode(*currentMode);
    }
    if (targetMode.width <= 0 || targetMode.height <= 0) {
        return;
    }

    windowMode_ = mode;
    glfwSetWindowMonitor(
        handle_.get(),
        monitor,
        x,
        y,
        targetMode.width,
        targetMode.height,
        targetMode.refreshRate);
}

bool GLFWNativeWindow::setIcon(std::span<const ImageRgba> images)
{
    if (images.empty()) {
        clearIcon();
        return true;
    }

    std::vector<GLFWimage> glfwImages;
    glfwImages.reserve(images.size());
    for (const ImageRgba& image : images) {
        std::optional<GLFWimage> glfwImage = toGlfwImage(image);
        if (!glfwImage) {
            return false;
        }
        glfwImages.push_back(*glfwImage);
    }

    clearGlfwError();
    glfwSetWindowIcon(handle_.get(), static_cast<int>(glfwImages.size()), glfwImages.data());
    return takeGlfwError().code == GLFW_NO_ERROR;
}

void GLFWNativeWindow::clearIcon()
{
    glfwSetWindowIcon(handle_.get(), 0, nullptr);
}

void GLFWNativeWindow::requestAttention()
{
    glfwRequestWindowAttention(handle_.get());
}

void GLFWNativeWindow::setFocus(bool focus) const noexcept
{
    if (focus) {
        glfwFocusWindow(handle_.get());
    }
}

void GLFWNativeWindow::setVisible(bool visible) const noexcept
{
    if (visible) {
        glfwShowWindow(handle_.get());
    } else {
        glfwHideWindow(handle_.get());
    }
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

std::pair<int, int> GLFWNativeWindow::getPosition() const noexcept
{
    int x, y;
    glfwGetWindowPos(handle_.get(), &x, &y);
    return {
        x,
        y,
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

std::pair<float, float> GLFWNativeWindow::getContentScale() const noexcept
{
    float xScale = 1.0f;
    float yScale = 1.0f;
    glfwGetWindowContentScale(handle_.get(), &xScale, &yScale);
    return {
        xScale,
        yScale,
    };
}

float GLFWNativeWindow::getOpacity() const noexcept
{
    return glfwGetWindowOpacity(handle_.get());
}

CursorMode GLFWNativeWindow::getCursorMode() const noexcept
{
    return cursorMode_;
}

bool GLFWNativeWindow::isRawMouseMotionEnabled() const noexcept
{
    return glfwGetInputMode(handle_.get(), GLFW_RAW_MOUSE_MOTION) == GLFW_TRUE;
}

WindowMode GLFWNativeWindow::getWindowMode() const noexcept
{
    return windowMode_;
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
    previousErrorCallback_ = glfwSetErrorCallback(glfwErrorCallback);
    clearGlfwError();

    if (!glfwInit()) {
        GLFWerrorfun previous = previousErrorCallback_;
        previousErrorCallback_ = nullptr;
        glfwSetErrorCallback(previous);
        throw makeGlfwError(ErrorCode::InitializationFailed, "Failed to initialize GLFW");
    }
}

GLFWWindowContext::~GLFWWindowContext()
{
    glfwTerminate();
    glfwSetErrorCallback(previousErrorCallback_);
}

void GLFWWindowContext::pollEvents() noexcept
{
    // clear old event buffers
    g_WindowRegistry.resetAll();
    // poll new events
    glfwPollEvents();
    pollJoysticks();
    pollGamepads();
}

void GLFWWindowContext::waitEvents() noexcept
{
    g_WindowRegistry.resetAll();
    glfwWaitEvents();
    pollJoysticks();
    pollGamepads();
}

void GLFWWindowContext::waitEventsTimeout(double timeoutSeconds) noexcept
{
    g_WindowRegistry.resetAll();
    glfwWaitEventsTimeout(std::max(0.0, timeoutSeconds));
    pollJoysticks();
    pollGamepads();
}

void GLFWWindowContext::postEmptyEvent() noexcept
{
    glfwPostEmptyEvent();
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
    if (!ext) {
        return {};
    }

    return std::vector<std::string>(ext, ext + count);
}

std::vector<MonitorInfo> GLFWWindowContext::getMonitors() const
{
    std::vector<MonitorInfo> infos;
    auto monitors = getOrderedMonitors();
    infos.reserve(monitors.size());

    for (size_t i = 0; i < monitors.size(); ++i) {
        infos.push_back(toMonitorInfo(monitors[i], static_cast<uint32_t>(i), i == 0));
    }

    return infos;
}

std::optional<MonitorInfo> GLFWWindowContext::getPrimaryMonitor() const
{
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    if (!primary) {
        return std::nullopt;
    }

    return toMonitorInfo(primary, 0, true);
}

std::vector<VideoMode> GLFWWindowContext::getVideoModes(uint32_t monitorId) const
{
    GLFWmonitor* monitor = getMonitorById(monitorId);
    if (!monitor) {
        return {};
    }

    int count = 0;
    const GLFWvidmode* modes = glfwGetVideoModes(monitor, &count);
    if (!modes || count <= 0) {
        return {};
    }

    std::vector<VideoMode> result;
    result.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        result.push_back(toVideoMode(modes[i]));
    }
    return result;
}

std::pair<float, float> GLFWWindowContext::getContentScale(uint32_t monitorId) const
{
    GLFWmonitor* monitor = getMonitorById(monitorId);
    if (!monitor) {
        return { 1.0f, 1.0f };
    }

    float xScale = 1.0f;
    float yScale = 1.0f;
    glfwGetMonitorContentScale(monitor, &xScale, &yScale);
    return { xScale, yScale };
}

std::vector<GamepadInfo> GLFWWindowContext::getGamepads() const
{
    std::vector<GamepadInfo> gamepads;
    gamepads.reserve(MaxGamepads);

    for (uint32_t id = 0; id < MaxGamepads; ++id) {
        if (auto info = readGamepadInfo(id)) {
            gamepads.push_back(std::move(*info));
        }
    }

    return gamepads;
}

std::optional<GamepadState> GLFWWindowContext::getGamepadState(uint32_t gamepadId) const
{
    return readStandardGamepadState(gamepadId);
}

bool GLFWWindowContext::isRawMouseMotionSupported() const
{
    return glfwRawMouseMotionSupported() == GLFW_TRUE;
}

bool GLFWWindowContext::setClipboardText(const std::string& text) const
{
    clearGlfwError();
    glfwSetClipboardString(nullptr, text.c_str());
    return takeGlfwError().code == GLFW_NO_ERROR;
}

std::string GLFWWindowContext::getClipboardText() const
{
    return tryGetClipboardText().value_or(std::string());
}

std::optional<std::string> GLFWWindowContext::tryGetClipboardText() const
{
    clearGlfwError();
    const char* text = glfwGetClipboardString(nullptr);
    if (text) {
        (void)takeGlfwError();
        return std::string(text);
    }

    return takeGlfwError().code == GLFW_NO_ERROR ? std::optional<std::string>{ std::string() }
                                                 : std::nullopt;
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
