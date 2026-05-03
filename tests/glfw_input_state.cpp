#include <cassert>

#include "backend/glfw/glfw_impl.hpp"

int main()
{
    cwin::GLFWInputState input;

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
    input.handleEvent(
        cwin::Event::MouseMoved{
            .posX = 10.0,
            .posY = 20.0,
        });
    auto mouseDelta = input.getMouseDelta();
    assert(mouseDelta.first == 0.0);
    assert(mouseDelta.second == 0.0);

    input.handleEvent(
        cwin::Event::MouseMoved{
            .posX = 15.0,
            .posY = 18.0,
        });
    mouseDelta = input.getMouseDelta();
    assert(mouseDelta.first == 5.0);
    assert(mouseDelta.second == -2.0);

    input.reset();
    mouseDelta = input.getMouseDelta();
    assert(mouseDelta.first == 0.0);
    assert(mouseDelta.second == 0.0);

    input.handleEvent(cwin::Event::MouseEntered{});
    assert(input.isMouseInside());

    input.handleEvent(cwin::Event::MouseLeft{});
    assert(!input.isMouseInside());

    input.reset();
    assert(!input.isKeyDown(cwin::Key::Unknown));
    assert(!input.isKeyDown(static_cast<cwin::Key>(9999)));
    assert(!input.isMouseButtonDown(cwin::MouseButton::Unknown));
    assert(!input.isMouseButtonDown(static_cast<cwin::MouseButton>(99)));
}
