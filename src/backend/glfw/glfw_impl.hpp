/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#ifndef CPPWINDOW_HEADER_GLFW_IMPL_HPP
#define CPPWINDOW_HEADER_GLFW_IMPL_HPP

#include <cppwindow/utils.hpp>

// Prevent GLFW from including OpenGL headers
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <bitset>
#include <format>
#include <stdexcept>
#include <string>

#include "../../window_registry.hpp"
#include "../native_impl.hpp"

class GLFWException : public std::runtime_error
{
public:
    // Accept a custom message, default to empty
    explicit GLFWException(const std::string& customMsg = "")
        : std::runtime_error(formatError(customMsg))
    {
    }

private:
    static std::string formatError(const std::string& customMsg)
    {
        const char* description = nullptr;
        int errorCode = glfwGetError(&description);

        std::string finalMsg = "GLFW Exception";
        if (!customMsg.empty()) {
            finalMsg += " [" + customMsg + "]";
        }

        if (description) {
            finalMsg +=
                ": " + std::string(description) + " (Code: " + std::to_string(errorCode) + ")";
        } else {
            finalMsg += ": No specific GLFW error reported.";
        }

        return finalMsg;
    }
};

namespace cwin {

//----------------------------------------------------------------------------
//  GLFW Input Mapping
//----------------------------------------------------------------------------
template <>
struct InputTraits<Key, int>
{
    static constexpr Key WrapperNone = Key::Unknown;
    static constexpr int BackendNone = -1;
    static constexpr int BackendMin = 0;
    static constexpr int BackendMax = GLFW_KEY_LAST + 1;
    static constexpr unsigned int WrapperMax = KeyCount;
};

template <>
struct InputTraits<MouseButton, int>
{
    static constexpr MouseButton WrapperNone = MouseButton::Unknown;
    static constexpr int BackendNone = -1;
    static constexpr int BackendMin = 0;
    static constexpr int BackendMax = GLFW_MOUSE_BUTTON_LAST;
    static constexpr unsigned int WrapperMax = MouseButtonCount;
};

using KeyMapLookup = StaticLookup<Key, int>;
using MouseMapLookup = StaticLookup<MouseButton, int>;

namespace inputmap {

int toGlfwKey(Key k);
Key toKey(int k);
int toGlfwMouseButton(MouseButton b);
MouseButton toMouseButton(int b);

}  // namespace inputmap

//----------------------------------------------------------------------------
//  GLFW Input State
//----------------------------------------------------------------------------
class GLFWInputState : public NativeInputState
{
    void handleEvent(const Event& event) override;
    void reset() override;

    // keyboard
    bool isKeyDown(Key key) const override;
    bool isKeyPressed(Key key) const override;
    bool isKeyReleased(Key key) const override;

    // mouse
    bool isMouseButtonDown(MouseButton button) const override;
    bool isMouseButtonPressed(MouseButton button) const override;
    bool isMouseButtonReleased(MouseButton button) const override;
    std::pair<double, double> getMousePosition() const override;
    std::pair<double, double> getScrollDelta() const override;

private:
    std::bitset<KeyCount> keyStates_{};
    std::bitset<KeyCount> prevKeyStates_{};

    std::bitset<MouseButtonCount> mouseStates_{};
    std::bitset<MouseButtonCount> prevMouseStates_{};

    double mousePosX_{}, mousePosY_{};
    double scrollDeltaX_{}, scrollDeltaY_{};
};

//----------------------------------------------------------------------------
//  GLFW Window
//----------------------------------------------------------------------------
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

class WindowStorage
{
public:
    std::vector<Event> eventQueue;
    std::unique_ptr<NativeInputState> inputState;

    void reset()
    {
        eventQueue.clear();
        inputState.reset();
    }
};

class GLFWNativeWindow : public NativeWindow
{
public:
    explicit GLFWNativeWindow(WindowDesc desc);
    ~GLFWNativeWindow() = default;

    void handleEvent(Event&& event);
    void registerCallbacks();

    NativeHandles getNativeHandles() const override;
    VulkanHandle createVulkanSurface(void* instance) const override;
    void makeContextCurrent() override;
    void swapBuffers() override;

    bool shouldClose() const noexcept override;
    void requestClose() noexcept override;

    std::span<Event> events() const noexcept override;
    const NativeInputState* getInput() const noexcept override;

    void setTitle(const std::string& title) override;
    void setSize(int width, int height) override;
    void setFocus(bool focus) const noexcept override;
    void setVisible(bool visible) const noexcept override;
    std::pair<int, int> getSize() const noexcept override;
    std::pair<uint32_t, uint32_t> getFrameBufferSize() const noexcept override;
    bool isFocused() const noexcept override;
    bool isVisible() const noexcept override;

private:
    UniqueGLFWwindow handle_{};
    std::shared_ptr<WindowStorage> storage_{};
};

//----------------------------------------------------------------------------
//  GLFW Context
//----------------------------------------------------------------------------
class GLFWWindowContext : public NativeWindowContext
{
public:
    GLFWWindowContext();
    ~GLFWWindowContext();

    void pollEvents() noexcept override;

    ProcLoader getProcLoader() const override;
    bool isVulkanSupported() const override;
    std::vector<std::string> getRequiredVulkanExtensions() const override;
};

}  // namespace cwin

#endif