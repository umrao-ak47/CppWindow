#include <cppwindow/cppwindow.hpp>

#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

#include "backend/glfw/glfw_input_state.hpp"

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
    auto scroll = input.scrollDelta();
    assert(scroll.first == 1.5);
    assert(scroll.second == 1.0);

    input.handleEvent(cwin::Event::MouseMoved{ .posX = 10.0, .posY = 20.0 });
    auto delta = input.mouseDelta();
    assert(delta.first == 0.0);
    assert(delta.second == 0.0);
    auto pos = input.mousePosition();
    assert(pos.first == 10.0);
    assert(pos.second == 20.0);

    input.handleEvent(cwin::Event::MouseMoved{ .posX = 15.0, .posY = 18.0 });
    input.handleEvent(cwin::Event::MouseMoved{ .posX = 18.5, .posY = 21.0 });
    delta = input.mouseDelta();
    assert(delta.first == 8.5);
    assert(delta.second == 1.0);

    input.reset();
    delta = input.mouseDelta();
    scroll = input.scrollDelta();
    assert(delta.first == 0.0);
    assert(delta.second == 0.0);
    assert(scroll.first == 0.0);
    assert(scroll.second == 0.0);

    input.setMousePosition(100.0, 50.0);
    pos = input.mousePosition();
    assert(pos.first == 100.0);
    assert(pos.second == 50.0);
    input.handleEvent(cwin::Event::MouseMoved{ .posX = 103.0, .posY = 44.0 });
    delta = input.mouseDelta();
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
    const cwin::ActionId jumpId = actions.defineAction("jump");
    const cwin::ActionId fireId = actions.defineAction("fire");
    const cwin::ActionId missingId{};
    actions.bindKey(jumpId, cwin::Key::Space)
        .bindKey(jumpId, cwin::Key::Space)
        .bindMouseButton(jumpId, cwin::MouseButton::Left)
        .bindMouseButton(jumpId, cwin::MouseButton::Left)
        .bindGamepadButton(jumpId, cwin::GamepadButton::A)
        .bindGamepadButton(jumpId, cwin::GamepadButton::A)
        .bindMouseButton(fireId, cwin::MouseButton::Right);

    const cwin::ActionBinding* jump = actions.binding(jumpId);
    assert(jump);
    assert(jump->keys.size() == 1);
    assert(jump->keys.front().key == cwin::Key::Space);
    assert(jump->mouseButtons.size() == 1);
    assert(jump->gamepadButtons.size() == 1);
    assert(!actions.binding(missingId));

    actions.update(input);
    assert(!actions.isDown(jumpId));
    assert(!actions.isPressed(jumpId));
    assert(!actions.isReleased(jumpId));
    assert(!actions.isDown(missingId));

    input.setKey(cwin::Key::Space, true);
    actions.update(input);
    assert(actions.isDown(jumpId));
    assert(actions.isPressed(jumpId));
    assert(!actions.isReleased(jumpId));

    actions.update(input);
    assert(actions.isDown(jumpId));
    assert(!actions.isPressed(jumpId));
    assert(!actions.isReleased(jumpId));

    input.setMouseButton(cwin::MouseButton::Left, true);
    actions.update(input);
    assert(actions.isDown(jumpId));
    assert(!actions.isPressed(jumpId));

    input.setKey(cwin::Key::Space, false);
    actions.update(input);
    assert(actions.isDown(jumpId));
    assert(!actions.isReleased(jumpId));

    input.setMouseButton(cwin::MouseButton::Left, false);
    actions.update(input);
    assert(!actions.isDown(jumpId));
    assert(!actions.isPressed(jumpId));
    assert(actions.isReleased(jumpId));

    gamepad.buttons[static_cast<std::size_t>(cwin::GamepadButton::A)] = true;
    actions.update(input, gamepad);
    assert(actions.isDown(jumpId));
    assert(actions.isPressed(jumpId));

    actions.resetState();
    assert(!actions.isDown(jumpId));
    assert(!actions.isPressed(jumpId));
    assert(!actions.isReleased(jumpId));

    actions.update(input, gamepad);
    assert(actions.isPressed(jumpId));

    input.setMouseButton(cwin::MouseButton::Right, true);
    actions.update(input, gamepad);
    assert(actions.isDown(fireId));
    assert(actions.isPressed(fireId));
    assert(actions.isDown(jumpId));
    assert(!actions.isPressed(jumpId));

    actions.clear(jumpId);
    assert(!actions.binding(jumpId));
    assert(!actions.isDown(jumpId));
    assert(actions.binding(fireId));

    actions.clearAll();
    assert(!actions.binding(fireId));
    assert(!actions.isDown(fireId));
}

void testActionMapIdsMetadataAndSnapshots()
{
    FakeInput input;
    cwin::GamepadState gamepad;
    cwin::ActionMap actions;

    const cwin::ActionId missing = actions.findAction("missing");
    assert(!missing);
    assert(!actions.hasAction(missing));
    assert(!actions.hasAction("jump"));

    const cwin::ActionId jump = actions.defineAction("jump");
    assert(jump);
    assert(actions.defineAction("jump") == jump);
    assert(actions.findAction("jump") == jump);
    assert(actions.hasAction("jump"));
    assert(actions.hasAction(jump));

    actions.setMetadata(
        jump,
        cwin::ActionMetadata{
            .displayName = "Jump",
            .description = "Leave the ground",
        });
    assert(actions.metadata(jump));
    assert(actions.metadata(jump)->displayName == "Jump");

    actions.bindKey(jump, cwin::Key::Space)
        .bindMouseButton(jump, cwin::MouseButton::Left)
        .bindGamepadButton(jump, cwin::GamepadButton::A)
        .bindGamepadAxis(jump, cwin::GamepadAxis::LeftX, 0.2f)
        .setContext(jump, "gameplay");

    const cwin::ActionBinding* binding = actions.binding(jump);
    assert(binding);
    assert(binding->context == "gameplay");
    assert(binding->keys.size() == 1);
    assert(binding->mouseButtons.size() == 1);
    assert(binding->gamepadButtons.size() == 1);
    assert(binding->gamepadAxes.size() == 1);

    input.setKey(cwin::Key::Space, true);
    actions.update(input, gamepad);
    assert(actions.isDown(jump));
    assert(actions.isPressed(jump));
    assert(!actions.isReleased(jump));

    std::vector<cwin::ActionInfo> infos = actions.actions();
    assert(infos.size() == 1);
    assert(infos.front().id == jump);
    assert(infos.front().name == "jump");
    assert(infos.front().metadata.description == "Leave the ground");
    assert(infos.front().pressed);
    assert(infos.front().down);

    input.setKey(cwin::Key::Space, false);
    gamepad.axes[static_cast<std::size_t>(cwin::GamepadAxis::LeftX)] = 0.7f;
    actions.update(input, gamepad);
    assert(actions.isDown(jump));
    assert(closeTo(actions.axisValue(jump), 0.7f));

    actions.replaceKey(jump, cwin::Key::F5)
        .replaceKeyCombo(jump, cwin::Key::A, { cwin::Key::LShift })
        .replaceMouseButton(jump, cwin::MouseButton::Right)
        .replaceGamepadButton(jump, cwin::GamepadButton::B)
        .replaceGamepadAxis(jump, cwin::GamepadAxis::RightY, -1.0f);

    binding = actions.binding(jump);
    assert(binding);
    assert(binding->keys.size() == 1);
    assert(binding->keys.front().key == cwin::Key::A);
    assert(binding->keys.front().requiredKeys.size() == 1);
    assert(binding->mouseButtons.front() == cwin::MouseButton::Right);
    assert(binding->gamepadButtons.front() == cwin::GamepadButton::B);
    assert(binding->gamepadAxes.front().axis == cwin::GamepadAxis::RightY);
    assert(closeTo(binding->gamepadAxes.front().deadzone, 0.0f));
    assert(!actions.isDown(jump));

    actions.clearBindings(jump);
    binding = actions.binding(jump);
    assert(binding);
    assert(binding->context == "gameplay");
    assert(binding->keys.empty());
    assert(binding->mouseButtons.empty());
    assert(binding->gamepadButtons.empty());
    assert(binding->gamepadAxes.empty());

    actions.setContextEnabled("gameplay", false);
    assert(!actions.isContextEnabled("gameplay"));
    actions.clearContextStates();
    assert(actions.isContextEnabled("gameplay"));

    const cwin::ActionId pause = actions.defineAction("pause");
    actions.setMetadata(
        pause,
        cwin::ActionMetadata{
            .displayName = "Pause",
            .description = "Open pause menu",
        });
    assert(pause);
    assert(pause != jump);
    assert(actions.metadata(pause)->displayName == "Pause");

    actions.clear(jump);
    assert(!actions.hasAction(jump));
    assert(actions.hasAction(pause));

    actions.clear(pause);
    assert(actions.actions().empty());

    actions.bindKey(cwin::ActionId{ 999 }, cwin::Key::Q)
        .replaceKey(cwin::ActionId{ 999 }, cwin::Key::W)
        .setMetadata(cwin::ActionId{ 999 }, cwin::ActionMetadata{});
    actions.clearBindings(cwin::ActionId{ 999 });
    actions.clear(cwin::ActionId{ 999 });
    assert(actions.actions().empty());
}

void testActionMapModifiers()
{
    FakeInput input;
    cwin::ActionMap actions;
    const cwin::ActionId saveId = actions.defineAction("save");
    const cwin::ActionId exactSaveId = actions.defineAction("exact_save");
    const cwin::ActionId shiftJumpId = actions.defineAction("shift_jump");
    const cwin::ActionId leftShiftAId = actions.defineAction("left_shift_a");
    const cwin::ActionId rightShiftAId = actions.defineAction("right_shift_a");
    actions.bindKey(saveId, cwin::Key::S, cwin::Modifiers{ .control = true })
        .bindKey(exactSaveId, cwin::Key::S, cwin::Modifiers{ .control = true }, true)
        .bindKey(shiftJumpId, cwin::Key::Space, cwin::Modifiers{ .shift = true })
        .bindKeyCombo(leftShiftAId, cwin::Key::A, { cwin::Key::LShift })
        .bindKeyCombo(rightShiftAId, cwin::Key::A, { cwin::Key::RShift });

    const cwin::ActionBinding* save = actions.binding(saveId);
    assert(save);
    assert(save->keys.size() == 1);
    assert(save->keys.front().modifiers.control);
    assert(!save->keys.front().exactModifiers);

    input.setKey(cwin::Key::S, true);
    actions.update(input);
    assert(!actions.isDown(saveId));

    input.setKey(cwin::Key::LControl, true);
    actions.update(input);
    assert(actions.isDown(saveId));
    assert(actions.isPressed(saveId));
    assert(actions.isDown(exactSaveId));

    input.setKey(cwin::Key::LShift, true);
    actions.update(input);
    assert(actions.isDown(saveId));
    assert(!actions.isDown(exactSaveId));

    input.setKey(cwin::Key::Space, true);
    actions.update(input);
    assert(actions.isDown(shiftJumpId));
    assert(!actions.isDown(leftShiftAId));
    assert(!actions.isDown(rightShiftAId));

    input.setKey(cwin::Key::A, true);
    actions.update(input);
    assert(actions.isDown(leftShiftAId));
    assert(!actions.isDown(rightShiftAId));

    input.setKey(cwin::Key::LShift, false);
    input.setKey(cwin::Key::RShift, true);
    actions.update(input);
    assert(!actions.isDown(leftShiftAId));
    assert(actions.isDown(rightShiftAId));

    actions.replaceKeyCombo(leftShiftAId, cwin::Key::B, { cwin::Key::LShift });
    const cwin::ActionBinding* chord = actions.binding(leftShiftAId);
    assert(chord);
    assert(chord->keys.size() == 1);
    assert(chord->keys.front().key == cwin::Key::B);
    assert(chord->keys.front().requiredKeys.size() == 1);
    assert(chord->keys.front().requiredKeys.front() == cwin::Key::LShift);

    actions.replaceKey(saveId, cwin::Key::F5);
    save = actions.binding(saveId);
    assert(save);
    assert(save->keys.size() == 1);
    assert(save->keys.front().key == cwin::Key::F5);
    assert(!save->keys.front().modifiers.any());

    actions.update(input);
    assert(!actions.isDown(saveId));
    input.setKey(cwin::Key::F5, true);
    actions.update(input);
    assert(actions.isDown(saveId));
}

void testActionMapAxesContextsAndRebinding()
{
    FakeInput input;
    cwin::GamepadState gamepad;
    cwin::ActionMap actions;
    const cwin::ActionId jumpId = actions.defineAction("jump");
    const cwin::ActionId fireId = actions.defineAction("fire");
    const cwin::ActionId moveXId = actions.defineAction("move_x");
    const cwin::ActionId moveLeftId = actions.defineAction("move_left");
    actions.bindKey(jumpId, cwin::Key::Space)
        .bindMouseButton(fireId, cwin::MouseButton::Left)
        .bindGamepadAxis(moveXId, cwin::GamepadAxis::LeftX, 0.25f)
        .bindGamepadAxis(moveLeftId, cwin::GamepadAxis::LeftX, 0.25f, cwin::AxisDirection::Negative)
        .setContext(jumpId, "gameplay")
        .setContext(fireId, "gameplay")
        .setContext(moveXId, "gameplay")
        .setContext(moveLeftId, "gameplay");

    const cwin::ActionBinding* move = actions.binding(moveXId);
    assert(move);
    assert(move->context == "gameplay");
    assert(move->gamepadAxes.size() == 1);
    assert(closeTo(move->gamepadAxes.front().deadzone, 0.25f));

    gamepad.axes[static_cast<std::size_t>(cwin::GamepadAxis::LeftX)] = 0.2f;
    actions.update(input, gamepad);
    assert(!actions.isDown(moveXId));
    assert(closeTo(actions.axisValue(moveXId), 0.0f));

    gamepad.axes[static_cast<std::size_t>(cwin::GamepadAxis::LeftX)] = 0.6f;
    actions.update(input, gamepad);
    assert(actions.isDown(moveXId));
    assert(actions.isPressed(moveXId));
    assert(closeTo(actions.axisValue(moveXId), 0.6f));
    assert(!actions.isDown(moveLeftId));

    gamepad.axes[static_cast<std::size_t>(cwin::GamepadAxis::LeftX)] = -0.7f;
    actions.update(input, gamepad);
    assert(actions.isDown(moveXId));
    assert(closeTo(actions.axisValue(moveXId), -0.7f));
    assert(actions.isDown(moveLeftId));
    assert(closeTo(actions.axisValue(moveLeftId), -0.7f));

    input.setKey(cwin::Key::Space, true);
    actions.update(input, gamepad);
    assert(actions.isDown(jumpId));

    actions.setContextEnabled("gameplay", false);
    assert(!actions.isContextEnabled("gameplay"));
    actions.update(input, gamepad);
    assert(!actions.isDown(jumpId));
    assert(actions.isReleased(jumpId));
    assert(!actions.isDown(moveXId));
    assert(closeTo(actions.axisValue(moveXId), 0.0f));

    actions.setContextEnabled("gameplay", true);
    actions.update(input, gamepad);
    assert(actions.isDown(jumpId));
    assert(actions.isPressed(jumpId));

    actions.replaceMouseButton(fireId, cwin::MouseButton::Right);
    const cwin::ActionBinding* fire = actions.binding(fireId);
    assert(fire);
    assert(fire->context == "gameplay");
    assert(fire->mouseButtons.size() == 1);
    assert(fire->mouseButtons.front() == cwin::MouseButton::Right);

    actions.replaceGamepadAxis(moveXId, cwin::GamepadAxis::RightX, 2.0f);
    move = actions.binding(moveXId);
    assert(move);
    assert(move->gamepadAxes.size() == 1);
    assert(closeTo(move->gamepadAxes.front().deadzone, 1.0f));

    actions.clearBindings(moveXId);
    move = actions.binding(moveXId);
    assert(move);
    assert(move->context == "gameplay");
    assert(move->gamepadAxes.empty());

    actions.clearContextStates();
    assert(actions.isContextEnabled("gameplay"));
}

}  // namespace

int main()
{
    testInputStateTransitions();
    testMouseState();
    testActionMapTransitions();
    testActionMapIdsMetadataAndSnapshots();
    testActionMapModifiers();
    testActionMapAxesContextsAndRebinding();
}
