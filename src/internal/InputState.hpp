/*
 * Copyright (c) 2026 Your Name
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#pragma once

#include <glfw-vk/Events.hpp>
#include <glfw-vk/Input.hpp>

#include <bitset>
#include <optional>
#include <queue>

#include "GlfwIncludes.hpp"
#include "InputMap.hpp"

namespace glfwvk {

class InputState : public Input
{
public:
    InputState() = default;
    ~InputState() = default;

    // The Hooks (Called by GLFW Callbacks)
    void handleFrameBufferResizeEvent(uint32_t width, uint32_t height);
    void handleCloseEvent();
    void handleResizeEvent(int width, int height);
    void handleKeyEvent(int glfwKey, int scancode, int action, int mods);
    void handleMouseEvent(int glfwBtn, int action, int mods);
    void handleScrollEvent(double offsetX, double offsetY);
    void handleCursorPosEvent(double posX, double posY);
    void handleFocusEvent(int focused);

    // keyboard
    bool isKeyDown(Key key) const override;
    bool isKeyPressed(Key key) const override;
    bool isKeyReleased(Key key) const override;

    // mouse
    bool isMouseButtonDown(MouseButton button) const override;
    bool isMouseButtonPressed(MouseButton button) const override;
    bool isMouseButtonReleased(MouseButton button) const override;
    std::pair<double, double> mousePosition() const override;
    std::pair<double, double> getScrollDelta() const override;

    std::optional<Event> popEvent();

    void pushEvent(Event&& event)
    {
        m_events.push(std::move(event));
    }

    void postUpdate();

private:
    std::bitset<KeyCount> m_keyState{};
    std::bitset<KeyCount> m_prevKeyState{};

    std::bitset<MouseButtonCount> m_mouseState{};
    std::bitset<MouseButtonCount> m_prevMouseState{};

    double m_mousePosX{}, m_mousePosY{};
    double m_scrollDeltaX{}, m_scrollDeltaY{};

    std::queue<Event> m_events{};
};

}  // namespace glfwvk