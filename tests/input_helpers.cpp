#include <cppwindow/cppwindow.hpp>

#include <array>
#include <cassert>
#include <cstddef>
#include <optional>
#include <type_traits>
#include <utility>

#include "backend/glfw/glfw_impl.hpp"

static_assert(std::is_same_v<
              decltype(std::declval<cwin::Window&>().setMousePosition(0.0, 0.0)),
              void>);
static_assert(std::is_same_v<decltype(std::declval<cwin::Window&>().setRawMouseMotion(true)), bool>);
static_assert(std::is_same_v<
              decltype(std::declval<const cwin::Window&>().isRawMouseMotionEnabled()),
              bool>);
static_assert(std::is_same_v<
              decltype(std::declval<cwin::WindowBuilder&>().rawMouseMotion()),
              cwin::WindowBuilder&>);
static_assert(std::is_same_v<
              decltype(std::declval<const cwin::WindowContext&>().isRawMouseMotionSupported()),
              bool>);

namespace {

class FakeInput final
{
public:
    void setKey(cwin::Key key, bool down)
    {
        const auto index = static_cast<std::size_t>(key);
        if (index < keys_.size()) {
            keys_[index] = down;
        }
    }

    void setMouseButton(cwin::MouseButton button, bool down)
    {
        const auto index = static_cast<std::size_t>(button);
        if (index < mouseButtons_.size()) {
            mouseButtons_[index] = down;
        }
    }

    bool isKeyDown(cwin::Key key) const
    {
        const auto index = static_cast<std::size_t>(key);
        return index < keys_.size() && keys_[index];
    }

    bool isMouseButtonDown(cwin::MouseButton button) const
    {
        const auto index = static_cast<std::size_t>(button);
        return index < mouseButtons_.size() && mouseButtons_[index];
    }

private:
    std::array<bool, cwin::KeyCount> keys_{};
    std::array<bool, cwin::MouseButtonCount> mouseButtons_{};
};

void testInputStateTransitions()
{
    cwin::GLFWInputState input;

    assert(!input.isKeyDown(cwin::Key::Unknown));
    assert(!input.isKeyPressed(static_cast<cwin::Key>(9999)));
    assert(!input.isMouseButtonDown(cwin::MouseButton::Unknown));
    assert(!input.isMouseButtonPressed(static_cast<cwin::MouseButton>(99)));

    input.handleEvent(cwin::Event::KeyPressed{ .key = cwin::Key::A, .scancode = 10 });
    assert(input.isKeyDown(cwin::Key::A));
    assert(input.isKeyPressed(cwin::Key::A));
    assert(!input.isKeyReleased(cwin::Key::A));

    input.reset();
    assert(input.isKeyDown(cwin::Key::A));
    assert(!input.isKeyPressed(cwin::Key::A));
    assert(!input.isKeyReleased(cwin::Key::A));

    input.handleEvent(cwin::Event::KeyPressed{ .key = cwin::Key::A, .scancode = 10 });
    assert(input.isKeyDown(cwin::Key::A));
    assert(!input.isKeyPressed(cwin::Key::A));

    input.handleEvent(cwin::Event::KeyReleased{ .key = cwin::Key::A, .scancode = 10 });
    assert(!input.isKeyDown(cwin::Key::A));
    assert(!input.isKeyPressed(cwin::Key::A));
    assert(input.isKeyReleased(cwin::Key::A));

    input.reset();
    assert(!input.isKeyReleased(cwin::Key::A));

    input.handleEvent(cwin::Event::MouseButtonPressed{ .button = cwin::MouseButton::Left });
    assert(input.isMouseButtonDown(cwin::MouseButton::Left));
    assert(input.isMouseButtonPressed(cwin::MouseButton::Left));
    assert(!input.isMouseButtonReleased(cwin::MouseButton::Left));

    input.reset();
    assert(input.isMouseButtonDown(cwin::MouseButton::Left));
    assert(!input.isMouseButtonPressed(cwin::MouseButton::Left));

    input.handleEvent(cwin::Event::MouseButtonReleased{ .button = cwin::MouseButton::Left });
    assert(!input.isMouseButtonDown(cwin::MouseButton::Left));
    assert(input.isMouseButtonReleased(cwin::MouseButton::Left));

    input.handleEvent(cwin::Event::KeyPressed{ .key = cwin::Key::B, .scancode = 11 });
    input.handleEvent(cwin::Event::MouseButtonPressed{ .button = cwin::MouseButton::Right });
    assert(input.isKeyDown(cwin::Key::B));
    assert(input.isMouseButtonDown(cwin::MouseButton::Right));
    input.handleEvent(cwin::Event::FocusLost{});
    assert(!input.isKeyDown(cwin::Key::B));
    assert(!input.isMouseButtonDown(cwin::MouseButton::Right));
}

void testMouseState()
{
    cwin::GLFWInputState input;

    input.handleEvent(cwin::Event::MouseWheelScrolled{ .deltaX = 1.0, .deltaY = -2.0 });
    input.handleEvent(cwin::Event::MouseWheelScrolled{ .deltaX = 0.5, .deltaY = 3.0 });
    auto scroll = input.getScrollDelta();
    assert(scroll.first == 1.5);
    assert(scroll.second == 1.0);

    input.handleEvent(cwin::Event::MouseMoved{ .posX = 10.0, .posY = 20.0 });
    auto delta = input.getMouseDelta();
    assert(delta.first == 0.0);
    assert(delta.second == 0.0);
    auto pos = input.getMousePosition();
    assert(pos.first == 10.0);
    assert(pos.second == 20.0);

    input.handleEvent(cwin::Event::MouseMoved{ .posX = 15.0, .posY = 18.0 });
    input.handleEvent(cwin::Event::MouseMoved{ .posX = 18.5, .posY = 21.0 });
    delta = input.getMouseDelta();
    assert(delta.first == 8.5);
    assert(delta.second == 1.0);

    input.reset();
    delta = input.getMouseDelta();
    scroll = input.getScrollDelta();
    assert(delta.first == 0.0);
    assert(delta.second == 0.0);
    assert(scroll.first == 0.0);
    assert(scroll.second == 0.0);

    input.setMousePosition(100.0, 50.0);
    pos = input.getMousePosition();
    assert(pos.first == 100.0);
    assert(pos.second == 50.0);
    input.handleEvent(cwin::Event::MouseMoved{ .posX = 103.0, .posY = 44.0 });
    delta = input.getMouseDelta();
    assert(delta.first == 3.0);
    assert(delta.second == -6.0);

    input.handleEvent(cwin::Event::MouseEntered{});
    assert(input.isMouseInside());
    input.handleEvent(cwin::Event::MouseLeft{});
    assert(!input.isMouseInside());
}

void testActionMapTransitions()
{
    FakeInput input;
    cwin::GamepadState gamepad;

    cwin::ActionMap actions;
    actions.bindKey("jump", cwin::Key::Space)
        .bindKey("jump", cwin::Key::Space)
        .bindMouseButton("jump", cwin::MouseButton::Left)
        .bindMouseButton("jump", cwin::MouseButton::Left)
        .bindGamepadButton("jump", cwin::GamepadButton::A)
        .bindGamepadButton("jump", cwin::GamepadButton::A)
        .bindMouseButton("fire", cwin::MouseButton::Right);

    const cwin::ActionBinding* jump = actions.getBinding("jump");
    assert(jump);
    assert(jump->keys.size() == 1);
    assert(jump->mouseButtons.size() == 1);
    assert(jump->gamepadButtons.size() == 1);
    assert(!actions.getBinding("missing"));

    actions.update(input);
    assert(!actions.isDown("jump"));
    assert(!actions.isPressed("jump"));
    assert(!actions.isReleased("jump"));
    assert(!actions.isDown("missing"));

    input.setKey(cwin::Key::Space, true);
    actions.update(input);
    assert(actions.isDown("jump"));
    assert(actions.isPressed("jump"));
    assert(!actions.isReleased("jump"));

    actions.update(input);
    assert(actions.isDown("jump"));
    assert(!actions.isPressed("jump"));
    assert(!actions.isReleased("jump"));

    input.setMouseButton(cwin::MouseButton::Left, true);
    actions.update(input);
    assert(actions.isDown("jump"));
    assert(!actions.isPressed("jump"));

    input.setKey(cwin::Key::Space, false);
    actions.update(input);
    assert(actions.isDown("jump"));
    assert(!actions.isReleased("jump"));

    input.setMouseButton(cwin::MouseButton::Left, false);
    actions.update(input);
    assert(!actions.isDown("jump"));
    assert(!actions.isPressed("jump"));
    assert(actions.isReleased("jump"));

    gamepad.buttons[static_cast<std::size_t>(cwin::GamepadButton::A)] = true;
    actions.update(input, gamepad);
    assert(actions.isDown("jump"));
    assert(actions.isPressed("jump"));

    actions.resetState();
    assert(!actions.isDown("jump"));
    assert(!actions.isPressed("jump"));
    assert(!actions.isReleased("jump"));

    actions.update(input, gamepad);
    assert(actions.isPressed("jump"));

    input.setMouseButton(cwin::MouseButton::Right, true);
    actions.update(input, gamepad);
    assert(actions.isDown("fire"));
    assert(actions.isPressed("fire"));
    assert(actions.isDown("jump"));
    assert(!actions.isPressed("jump"));

    actions.clear("jump");
    assert(!actions.getBinding("jump"));
    assert(!actions.isDown("jump"));
    assert(actions.getBinding("fire"));

    actions.clearAll();
    assert(!actions.getBinding("fire"));
    assert(!actions.isDown("fire"));
}

}  // namespace

int main()
{
    testInputStateTransitions();
    testMouseState();
    testActionMapTransitions();
}
