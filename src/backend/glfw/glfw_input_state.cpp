/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#include "glfw_input_state.hpp"

#include <GLFW/glfw3.h>
#include <cstddef>
#include <type_traits>

namespace cwin::backend::glfw {

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

    glfwGetCursorPos(window, &data_.mousePositionX, &data_.mousePositionY);
    data_.hasMousePosition = true;
    data_.mouseInside = glfwGetWindowAttrib(window, GLFW_HOVERED) == GLFW_TRUE;
}

bool GLFWInputState::queryMouseInside() const noexcept
{
    if (!window_) {
        return data_.mouseInside;
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
                data_.keyStates.set(static_cast<size_t>(event.key));
            }
        } else if constexpr (std::is_same_v<T, Event::KeyReleased>) {
            if (isValidKey(event.key)) {
                data_.keyStates.reset(static_cast<size_t>(event.key));
            }
        } else if constexpr (std::is_same_v<T, Event::MouseButtonPressed>) {
            if (isValidMouseButton(event.button)) {
                data_.mouseButtonStates.set(static_cast<size_t>(event.button));
            }
        } else if constexpr (std::is_same_v<T, Event::MouseButtonReleased>) {
            if (isValidMouseButton(event.button)) {
                data_.mouseButtonStates.reset(static_cast<size_t>(event.button));
            }
        } else if constexpr (std::is_same_v<T, Event::MouseWheelScrolled>) {
            data_.scrollDeltaX += event.deltaX;
            data_.scrollDeltaY += event.deltaY;
        } else if constexpr (std::is_same_v<T, Event::MouseMoved>) {
            if (data_.hasMousePosition) {
                data_.mouseDeltaX += event.posX - data_.mousePositionX;
                data_.mouseDeltaY += event.posY - data_.mousePositionY;
            }
            data_.mousePositionX = event.posX;
            data_.mousePositionY = event.posY;
            data_.hasMousePosition = true;
        } else if constexpr (std::is_same_v<T, Event::MouseEntered>) {
            data_.mouseInside = true;
        } else if constexpr (std::is_same_v<T, Event::MouseLeft>) {
            data_.mouseInside = false;
        } else if constexpr (std::is_same_v<T, Event::FocusLost>) {
            data_.keyStates.reset();
            data_.mouseButtonStates.reset();
        }
    });
}

bool GLFWInputState::isKeyDown(Key key) const
{
    return data_.isKeyDown(key);
}

bool GLFWInputState::isKeyPressed(Key key) const
{
    return data_.isKeyPressed(key);
}

bool GLFWInputState::isKeyReleased(Key key) const
{
    return data_.isKeyReleased(key);
}

bool GLFWInputState::isMouseButtonDown(MouseButton button) const
{
    return data_.isMouseButtonDown(button);
}

bool GLFWInputState::isMouseButtonPressed(MouseButton button) const
{
    return data_.isMouseButtonPressed(button);
}

bool GLFWInputState::isMouseButtonReleased(MouseButton button) const
{
    return data_.isMouseButtonReleased(button);
}

std::pair<double, double> GLFWInputState::mousePosition() const
{
    return data_.mousePosition();
}

void GLFWInputState::setMousePosition(double x, double y)
{
    data_.mousePositionX = x;
    data_.mousePositionY = y;
    data_.hasMousePosition = true;
}

std::pair<double, double> GLFWInputState::mouseDelta() const
{
    return data_.mouseDelta();
}

std::pair<double, double> GLFWInputState::scrollDelta() const
{
    return data_.scrollDelta();
}

bool GLFWInputState::isMouseInside() const
{
    return queryMouseInside();
}

void GLFWInputState::reset() noexcept
{
    data_.previousKeyStates = data_.keyStates;
    data_.previousMouseButtonStates = data_.mouseButtonStates;
    data_.mouseDeltaX = 0;
    data_.mouseDeltaY = 0;
    data_.scrollDeltaX = 0;
    data_.scrollDeltaY = 0;
    data_.mouseInside = queryMouseInside();
}

const InputStateData& GLFWInputState::data() const noexcept
{
    return data_;
}

}  // namespace cwin::backend::glfw
