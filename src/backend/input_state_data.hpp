/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#ifndef CPPWINDOW_HEADER_INPUT_STATE_DATA_HPP
#define CPPWINDOW_HEADER_INPUT_STATE_DATA_HPP

#include <cppwindow/input.hpp>

#include <bitset>
#include <cstddef>
#include <utility>

namespace cwin::backend {

struct InputStateData
{
    [[nodiscard]] bool isKeyDown(Key key) const noexcept
    {
        const auto index = static_cast<std::size_t>(key);
        return index > static_cast<std::size_t>(Key::Unknown) && index < KeyCount &&
               keyStates.test(index);
    }

    [[nodiscard]] bool isKeyPressed(Key key) const noexcept
    {
        const auto index = static_cast<std::size_t>(key);
        return index > static_cast<std::size_t>(Key::Unknown) && index < KeyCount &&
               keyStates.test(index) && !previousKeyStates.test(index);
    }

    [[nodiscard]] bool isKeyReleased(Key key) const noexcept
    {
        const auto index = static_cast<std::size_t>(key);
        return index > static_cast<std::size_t>(Key::Unknown) && index < KeyCount &&
               !keyStates.test(index) && previousKeyStates.test(index);
    }

    [[nodiscard]] bool isMouseButtonDown(MouseButton button) const noexcept
    {
        const auto index = static_cast<std::size_t>(button);
        return index > static_cast<std::size_t>(MouseButton::Unknown) && index < MouseButtonCount &&
               mouseButtonStates.test(index);
    }

    [[nodiscard]] bool isMouseButtonPressed(MouseButton button) const noexcept
    {
        const auto index = static_cast<std::size_t>(button);
        return index > static_cast<std::size_t>(MouseButton::Unknown) && index < MouseButtonCount &&
               mouseButtonStates.test(index) && !previousMouseButtonStates.test(index);
    }

    [[nodiscard]] bool isMouseButtonReleased(MouseButton button) const noexcept
    {
        const auto index = static_cast<std::size_t>(button);
        return index > static_cast<std::size_t>(MouseButton::Unknown) && index < MouseButtonCount &&
               !mouseButtonStates.test(index) && previousMouseButtonStates.test(index);
    }

    [[nodiscard]] std::pair<double, double> mousePosition() const noexcept
    {
        return { mousePositionX, mousePositionY };
    }

    [[nodiscard]] std::pair<double, double> mouseDelta() const noexcept
    {
        return { mouseDeltaX, mouseDeltaY };
    }

    [[nodiscard]] std::pair<double, double> scrollDelta() const noexcept
    {
        return { scrollDeltaX, scrollDeltaY };
    }

    std::bitset<KeyCount> keyStates{};
    std::bitset<KeyCount> previousKeyStates{};
    std::bitset<MouseButtonCount> mouseButtonStates{};
    std::bitset<MouseButtonCount> previousMouseButtonStates{};
    double mousePositionX = 0.0;
    double mousePositionY = 0.0;
    double mouseDeltaX = 0.0;
    double mouseDeltaY = 0.0;
    double scrollDeltaX = 0.0;
    double scrollDeltaY = 0.0;
    bool hasMousePosition = false;
    bool mouseInside = false;
};

}  // namespace cwin::backend

#endif
