/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#pragma once

#include <glfw-vk/KeyDefs.hpp>

#include <utility>

namespace glfwvk {

class Input
{
public:
    virtual ~Input() = default;

    // keyboard
    virtual bool isKeyDown(Key key) const = 0;
    virtual bool isKeyPressed(Key key) const = 0;
    virtual bool isKeyReleased(Key key) const = 0;

    // mouse
    virtual bool isMouseButtonDown(MouseButton button) const = 0;
    virtual bool isMouseButtonPressed(MouseButton button) const = 0;
    virtual bool isMouseButtonReleased(MouseButton button) const = 0;
    virtual std::pair<double, double> mousePosition() const = 0;
    virtual std::pair<double, double> getScrollDelta() const = 0;
};

}  // namespace glfwvk
