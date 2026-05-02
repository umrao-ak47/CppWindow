#include <cassert>
#include <memory>

#include "backend/glfw/glfw_impl.hpp"

int main()
{
    auto state = std::make_unique<cwin::GLFWInputState>();
    cwin::NativeInputState& input = *state;

    input.handleEvent(
        cwin::Event::KeyPressed{
            .key = cwin::Key::A,
            .scancode = 0,
        });
    assert(input.isKeyDown(cwin::Key::A));
    assert(input.isKeyPressed(cwin::Key::A));

    input.reset();
    assert(input.isKeyDown(cwin::Key::A));
    assert(!input.isKeyPressed(cwin::Key::A));

    input.handleEvent(
        cwin::Event::KeyReleased{
            .key = cwin::Key::A,
            .scancode = 0,
        });
    assert(!input.isKeyDown(cwin::Key::A));
    assert(input.isKeyReleased(cwin::Key::A));

    input.reset();
    assert(!input.isKeyReleased(cwin::Key::A));

    input.handleEvent(
        cwin::Event::MouseButtonPressed{
            .button = cwin::MouseButton::Left,
        });
    assert(input.isMouseButtonDown(cwin::MouseButton::Left));
    assert(input.isMouseButtonPressed(cwin::MouseButton::Left));

    input.reset();
    input.handleEvent(
        cwin::Event::MouseWheelScrolled{
            .deltaX = 1.0,
            .deltaY = -2.0,
        });
    const auto scrollDelta = input.getScrollDelta();
    assert(scrollDelta.first == 1.0);
    assert(scrollDelta.second == -2.0);

    input.reset();
    assert(!input.isKeyDown(cwin::Key::Unknown));
    assert(!input.isKeyDown(static_cast<cwin::Key>(9999)));
    assert(!input.isMouseButtonDown(cwin::MouseButton::Unknown));
    assert(!input.isMouseButtonDown(static_cast<cwin::MouseButton>(99)));
}
