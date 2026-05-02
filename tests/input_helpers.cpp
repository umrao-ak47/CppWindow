#include <cppwindow/cppwindow.hpp>

#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <optional>
#include <type_traits>
#include <utility>

#include "backend/glfw/glfw_impl.hpp"

static_assert(
    std::is_same_v<decltype(std::declval<cwin::Window&>().setMousePosition(0.0, 0.0)), void>);
static_assert(
    std::is_same_v<decltype(std::declval<cwin::Window&>().setRawMouseMotion(true)), bool>);
static_assert(
    std::is_same_v<decltype(std::declval<const cwin::Window&>().isRawMouseMotionEnabled()), bool>);
static_assert(std::is_same_v<
              decltype(std::declval<cwin::WindowBuilder&>().rawMouseMotion()),
              cwin::WindowBuilder&>);
static_assert(std::is_same_v<
              decltype(std::declval<const cwin::WindowContext&>().isRawMouseMotionSupported()),
              bool>);

namespace {

[[nodiscard]] bool closeTo(float actual, float expected)
{
    return std::abs(actual - expected) < 0.000001f;
}

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
    assert(jump->keys.front().key == cwin::Key::Space);
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

void testActionMapModifiers()
{
    FakeInput input;
    cwin::ActionMap actions;
    actions.bindKey("save", cwin::Key::S, cwin::Modifiers{ .control = true })
        .bindKey("exact_save", cwin::Key::S, cwin::Modifiers{ .control = true }, true)
        .bindKey("shift_jump", cwin::Key::Space, cwin::Modifiers{ .shift = true })
        .bindKeyChord("left_shift_a", cwin::Key::A, { cwin::Key::LShift })
        .bindKeyChord("right_shift_a", cwin::Key::A, { cwin::Key::RShift });

    const cwin::ActionBinding* save = actions.getBinding("save");
    assert(save);
    assert(save->keys.size() == 1);
    assert(save->keys.front().modifiers.control);
    assert(!save->keys.front().exactModifiers);

    input.setKey(cwin::Key::S, true);
    actions.update(input);
    assert(!actions.isDown("save"));

    input.setKey(cwin::Key::LControl, true);
    actions.update(input);
    assert(actions.isDown("save"));
    assert(actions.isPressed("save"));
    assert(actions.isDown("exact_save"));

    input.setKey(cwin::Key::LShift, true);
    actions.update(input);
    assert(actions.isDown("save"));
    assert(!actions.isDown("exact_save"));

    input.setKey(cwin::Key::Space, true);
    actions.update(input);
    assert(actions.isDown("shift_jump"));
    assert(!actions.isDown("left_shift_a"));
    assert(!actions.isDown("right_shift_a"));

    input.setKey(cwin::Key::A, true);
    actions.update(input);
    assert(actions.isDown("left_shift_a"));
    assert(!actions.isDown("right_shift_a"));

    input.setKey(cwin::Key::LShift, false);
    input.setKey(cwin::Key::RShift, true);
    actions.update(input);
    assert(!actions.isDown("left_shift_a"));
    assert(actions.isDown("right_shift_a"));

    actions.replaceKeyChord("left_shift_a", cwin::Key::B, { cwin::Key::LShift });
    const cwin::ActionBinding* chord = actions.getBinding("left_shift_a");
    assert(chord);
    assert(chord->keys.size() == 1);
    assert(chord->keys.front().key == cwin::Key::B);
    assert(chord->keys.front().requiredKeys.size() == 1);
    assert(chord->keys.front().requiredKeys.front() == cwin::Key::LShift);

    actions.replaceKey("save", cwin::Key::F5);
    save = actions.getBinding("save");
    assert(save);
    assert(save->keys.size() == 1);
    assert(save->keys.front().key == cwin::Key::F5);
    assert(!save->keys.front().modifiers.any());

    actions.update(input);
    assert(!actions.isDown("save"));
    input.setKey(cwin::Key::F5, true);
    actions.update(input);
    assert(actions.isDown("save"));
}

void testActionMapAxesGroupsAndRebinding()
{
    FakeInput input;
    cwin::GamepadState gamepad;
    cwin::ActionMap actions;
    actions.bindKey("jump", cwin::Key::Space)
        .bindMouseButton("fire", cwin::MouseButton::Left)
        .bindGamepadAxis("move_x", cwin::GamepadAxis::LeftX, 0.25f)
        .bindGamepadAxis(
            "move_left",
            cwin::GamepadAxis::LeftX,
            0.25f,
            cwin::ActionAxisDirection::Negative)
        .setGroup("jump", "gameplay")
        .setGroup("fire", "gameplay")
        .setGroup("move_x", "gameplay")
        .setGroup("move_left", "gameplay");

    const cwin::ActionBinding* move = actions.getBinding("move_x");
    assert(move);
    assert(move->group == "gameplay");
    assert(move->gamepadAxes.size() == 1);
    assert(closeTo(move->gamepadAxes.front().deadzone, 0.25f));

    gamepad.axes[static_cast<std::size_t>(cwin::GamepadAxis::LeftX)] = 0.2f;
    actions.update(input, gamepad);
    assert(!actions.isDown("move_x"));
    assert(closeTo(actions.getAxis("move_x"), 0.0f));

    gamepad.axes[static_cast<std::size_t>(cwin::GamepadAxis::LeftX)] = 0.6f;
    actions.update(input, gamepad);
    assert(actions.isDown("move_x"));
    assert(actions.isPressed("move_x"));
    assert(closeTo(actions.getAxis("move_x"), 0.6f));
    assert(!actions.isDown("move_left"));

    gamepad.axes[static_cast<std::size_t>(cwin::GamepadAxis::LeftX)] = -0.7f;
    actions.update(input, gamepad);
    assert(actions.isDown("move_x"));
    assert(closeTo(actions.getAxis("move_x"), -0.7f));
    assert(actions.isDown("move_left"));
    assert(closeTo(actions.getAxis("move_left"), -0.7f));

    input.setKey(cwin::Key::Space, true);
    actions.update(input, gamepad);
    assert(actions.isDown("jump"));

    actions.setGroupEnabled("gameplay", false);
    assert(!actions.isGroupEnabled("gameplay"));
    actions.update(input, gamepad);
    assert(!actions.isDown("jump"));
    assert(actions.isReleased("jump"));
    assert(!actions.isDown("move_x"));
    assert(closeTo(actions.getAxis("move_x"), 0.0f));

    actions.setGroupEnabled("gameplay", true);
    actions.update(input, gamepad);
    assert(actions.isDown("jump"));
    assert(actions.isPressed("jump"));

    actions.replaceMouseButton("fire", cwin::MouseButton::Right);
    const cwin::ActionBinding* fire = actions.getBinding("fire");
    assert(fire);
    assert(fire->group == "gameplay");
    assert(fire->mouseButtons.size() == 1);
    assert(fire->mouseButtons.front() == cwin::MouseButton::Right);

    actions.replaceGamepadAxis("move_x", cwin::GamepadAxis::RightX, 2.0f);
    move = actions.getBinding("move_x");
    assert(move);
    assert(move->gamepadAxes.size() == 1);
    assert(closeTo(move->gamepadAxes.front().deadzone, 1.0f));

    actions.clearBindings("move_x");
    move = actions.getBinding("move_x");
    assert(move);
    assert(move->group == "gameplay");
    assert(move->gamepadAxes.empty());

    actions.clearGroupStates();
    assert(actions.isGroupEnabled("gameplay"));
}

}  // namespace

int main()
{
    testInputStateTransitions();
    testMouseState();
    testActionMapTransitions();
    testActionMapModifiers();
    testActionMapAxesGroupsAndRebinding();
}
