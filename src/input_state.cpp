/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#include <cppwindow/cppwindow.hpp>

#include "backend/input_state_data.hpp"

namespace cwin {

//----------------------------------------------------------------------------
//  Input State Implemenation
//----------------------------------------------------------------------------
InputState::InputState(const InputStateData* state)
    : state_(state)
{
}

bool InputState::isKeyDown(Key key) const
{
    return state_->isKeyDown(key);
}

bool InputState::isKeyPressed(Key key) const
{
    return state_->isKeyPressed(key);
}

bool InputState::isKeyReleased(Key key) const
{
    return state_->isKeyReleased(key);
}

bool InputState::isMouseButtonDown(MouseButton button) const
{
    return state_->isMouseButtonDown(button);
}

bool InputState::isMouseButtonPressed(MouseButton button) const
{
    return state_->isMouseButtonPressed(button);
}

bool InputState::isMouseButtonReleased(MouseButton button) const
{
    return state_->isMouseButtonReleased(button);
}

std::pair<double, double> InputState::mousePosition() const
{
    return state_->mousePosition();
}

std::pair<double, double> InputState::mouseDelta() const
{
    return state_->mouseDelta();
}

std::pair<double, double> InputState::scrollDelta() const
{
    return state_->scrollDelta();
}

bool InputState::isMouseInside() const
{
    return state_->mouseInside;
}

}  // namespace cwin
