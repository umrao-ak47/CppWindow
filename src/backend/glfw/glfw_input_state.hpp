/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#ifndef CPPWINDOW_HEADER_GLFW_INPUT_STATE_HPP
#define CPPWINDOW_HEADER_GLFW_INPUT_STATE_HPP

#include <cppwindow/events.hpp>

#include <utility>

#include "../input_state_data.hpp"

struct GLFWwindow;

namespace cwin::backend::glfw {

class GLFWInputState
{
public:
    GLFWInputState() = default;
    explicit GLFWInputState(GLFWwindow* window);

    void handleEvent(const Event& event);
    void reset() noexcept;

    // keyboard
    bool isKeyDown(Key key) const;
    bool isKeyPressed(Key key) const;
    bool isKeyReleased(Key key) const;

    // mouse
    bool isMouseButtonDown(MouseButton button) const;
    bool isMouseButtonPressed(MouseButton button) const;
    bool isMouseButtonReleased(MouseButton button) const;
    std::pair<double, double> mousePosition() const;
    void setMousePosition(double x, double y);
    std::pair<double, double> mouseDelta() const;
    std::pair<double, double> scrollDelta() const;
    bool isMouseInside() const;

    [[nodiscard]] const InputStateData& data() const noexcept;

private:
    bool queryMouseInside() const noexcept;

    GLFWwindow* window_ = nullptr;
    InputStateData data_{};
};

}  // namespace cwin::backend::glfw

#endif
