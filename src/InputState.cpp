#include "internal/InputState.hpp"
#include <glfw-vk/Events.hpp>

#include <cstdint>
#include <unordered_map>

namespace glfwvk {

void InputState::handleFrameBufferResizeEvent(uint32_t width, uint32_t height)
{
    if (width == 0 || height == 0) {
        return;
    }
    m_events.push(
        Event::FrameBufferResized{
            .width = width,
            .height = height,
        });
}

void InputState::handleCloseEvent()
{
    m_events.push(Event::Closed{});
}

void InputState::handleResizeEvent(int width, int height)
{
    m_events.push(
        Event::Resized{
            .width = width,
            .height = height,
        });
}

void InputState::handleKeyEvent(int glfwKey, int scancode, int action, int mods)
{
    Key key = InputMap::toKey(glfwKey);
    if (key != Key::Unknown) {
        if (action == GLFW_PRESS) {
            m_keyState.set(static_cast<size_t>(key));
            m_events.emplace(
                Event::KeyPressed{
                    .key = key,
                    .scancode = scancode,
                    .alt = (mods & GLFW_MOD_ALT) != 0,
                    .control = (mods & GLFW_MOD_CONTROL) != 0,
                    .shift = (mods & GLFW_MOD_SHIFT) != 0,
                    .system = (mods & GLFW_MOD_SUPER) != 0,
                });
        } else if (action == GLFW_RELEASE) {
            m_keyState.reset(static_cast<size_t>(key));
            m_events.emplace(
                Event::KeyReleased{
                    .key = key,
                    .scancode = scancode,
                    .alt = (mods & GLFW_MOD_ALT) != 0,
                    .control = (mods & GLFW_MOD_CONTROL) != 0,
                    .shift = (mods & GLFW_MOD_SHIFT) != 0,
                    .system = (mods & GLFW_MOD_SUPER) != 0,
                });
        }
    }
}

void InputState::handleMouseEvent(int glfwBtn, int action, int mods)
{
    MouseButton button = InputMap::toMouseButton(glfwBtn);
    if (action == GLFW_PRESS) {
        m_mouseState.set(static_cast<size_t>(button));
        m_events.push(
            Event::MouseButtonPressed{
                .button = button,
                .posX = m_mousePosX,
                .posY = m_mousePosY,
                .alt = (mods & GLFW_MOD_ALT) != 0,
                .control = (mods & GLFW_MOD_CONTROL) != 0,
                .shift = (mods & GLFW_MOD_SHIFT) != 0,
                .system = (mods & GLFW_MOD_SUPER) != 0,
            });
    } else if (action == GLFW_RELEASE) {
        m_mouseState.reset(static_cast<size_t>(button));
        m_events.push(
            Event::MouseButtonReleased{
                .button = button,
                .posX = m_mousePosX,
                .posY = m_mousePosY,
                .alt = (mods & GLFW_MOD_ALT) != 0,
                .control = (mods & GLFW_MOD_CONTROL) != 0,
                .shift = (mods & GLFW_MOD_SHIFT) != 0,
                .system = (mods & GLFW_MOD_SUPER) != 0,
            });
    }
}

void InputState::handleScrollEvent(double offsetX, double offsetY)
{
    m_scrollDeltaX += offsetX;
    m_scrollDeltaY += offsetY;
    m_events.push(
        Event::MouseWheelScrolled{
            .deltaX = m_scrollDeltaX,
            .deltaY = m_scrollDeltaY,
            .posX = m_mousePosX,
            .posY = m_mousePosY,
        });
}

void InputState::handleCursorPosEvent(double posX, double posY)
{
    m_mousePosX = posX;
    m_mousePosY = posY;
    m_events.push(
        Event::MouseMoved{
            .posX = m_mousePosX,
            .posY = m_mousePosY,
        });
}

void InputState::handleFocusEvent(int focused)
{
    if (focused == GLFW_FALSE) {
        m_keyState.reset();
        m_mouseState.reset();
    }
}

bool InputState::isKeyDown(Key key) const
{
    return m_keyState.test(static_cast<size_t>(key));
}

bool InputState::isKeyPressed(Key key) const
{
    size_t idx = static_cast<size_t>(key);
    return m_keyState.test(idx) && !m_prevKeyState.test(idx);
}

bool InputState::isKeyReleased(Key key) const
{
    size_t idx = static_cast<size_t>(key);
    return !m_keyState.test(idx) && m_prevKeyState.test(idx);
}

bool InputState::isMouseButtonDown(MouseButton button) const
{
    return m_mouseState.test(static_cast<size_t>(button));
}

bool InputState::isMouseButtonPressed(MouseButton button) const
{
    size_t idx = static_cast<size_t>(button);
    return m_mouseState.test(idx) && !m_prevMouseState.test(idx);
}

bool InputState::isMouseButtonReleased(MouseButton button) const
{
    size_t idx = static_cast<size_t>(button);
    return !m_mouseState.test(idx) && m_prevMouseState.test(idx);
}

std::pair<double, double> InputState::mousePosition() const
{
    return { m_mousePosX, m_mousePosY };
}

std::pair<double, double> InputState::getScrollDelta() const
{
    return { m_scrollDeltaX, m_scrollDeltaY };
}

std::optional<Event> InputState::popEvent()
{
    std::optional<Event> event;
    if (m_events.empty()) {
        return event;
    }
    event = m_events.front();
    m_events.pop();
    return event;
}

void InputState::postUpdate()
{
    m_prevKeyState = m_keyState;
    m_prevMouseState = m_mouseState;
    m_scrollDeltaX = 0;
    m_scrollDeltaY = 0;
}

}  // namespace glfwvk