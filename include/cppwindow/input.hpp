/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

/// @file input.hpp
/// @brief Input codes, input snapshots, gamepad state, and action bindings.

#ifndef CPPWINDOW_HEADER_INPUT_HPP
#define CPPWINDOW_HEADER_INPUT_HPP

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cwin {

/// Keyboard key code.
enum class Key : uint32_t
{
    Unknown = 0,
    First = 1,
    Space = First,
    Apostrophe,
    Comma,
    Minus,
    Period,
    Slash,
    Num0,
    Num1,
    Num2,
    Num3,
    Num4,
    Num5,
    Num6,
    Num7,
    Num8,
    Num9,
    Semicolon,
    Equal,
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    LBracket,
    Backslash,
    RBracket,
    Grave,
    World1,
    World2,
    Escape,
    Enter,
    Tab,
    Backspace,
    Insert,
    Delete,
    Right,
    Left,
    Down,
    Up,
    PageUp,
    PageDown,
    Home,
    End,
    CapsLock,
    ScrollLock,
    NumLock,
    PrintScreen,
    Pause,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    F13,
    F14,
    F15,
    F16,
    F17,
    F18,
    F19,
    F20,
    F21,
    F22,
    F23,
    F24,
    F25,
    Numpad0,
    Numpad1,
    Numpad2,
    Numpad3,
    Numpad4,
    Numpad5,
    Numpad6,
    Numpad7,
    Numpad8,
    Numpad9,
    NumpadDecimal,
    NumpadDivide,
    NumpadMultiply,
    NumpadSubtract,
    NumpadAdd,
    NumpadEnter,
    NumpadEqual,
    LShift,
    LControl,
    LAlt,
    LSuper,
    RShift,
    RControl,
    RAlt,
    RSuper,
    Menu,
    Last = Menu,
};

/// Backend-style input action names.
enum class Action : int
{
    /// Release action.
    Release = 0,
    /// Press action.
    Press,
    /// Repeat action.
    Repeat
};

/// Keyboard or mouse modifier state.
struct Modifiers
{
    /// Whether Alt/Option was held.
    bool alt = false;
    /// Whether Control was held.
    bool control = false;
    /// Whether Shift was held.
    bool shift = false;
    /// Whether the system/super/command modifier was held.
    bool system = false;

    /// Returns whether any modifier is held.
    [[nodiscard]] bool any() const noexcept
    {
        return alt || control || shift || system;
    }

    /// Compares two modifier sets.
    [[nodiscard]] bool operator==(const Modifiers&) const noexcept = default;
};

/// Mouse button code.
enum class MouseButton : uint8_t
{
    /// Unknown mouse button.
    Unknown = 0,
    /// First valid mouse button value.
    First = 1,
    Left = First,   //!< The left mouse button
    Right,          //!< The right mouse button
    Middle,         //!< The middle (wheel) mouse button
    Button4,        //!< The first extra mouse button
    Button5,        //!< The second extra mouse button
    Button6,        //!< The third extra mouse button
    Button7,        //!< The fourth extra mouse button
    Button8,        //!< The fifth extra mouse button
    Last = Button8  // Count for MouseButton
};

/// Number of key slots, including `Key::Unknown`.
inline constexpr std::size_t KeyCount{ static_cast<std::size_t>(Key::Last) + 1 };
/// Number of mouse button slots, including `MouseButton::Unknown`.
inline constexpr std::size_t MouseButtonCount{ static_cast<std::size_t>(MouseButton::Last) + 1 };

/// Standard gamepad button code.
enum class GamepadButton : uint8_t
{
    /// South face button on most controllers.
    A,
    /// East face button on most controllers.
    B,
    /// West face button on most controllers.
    X,
    /// North face button on most controllers.
    Y,
    /// Left shoulder button.
    LeftBumper,
    /// Right shoulder button.
    RightBumper,
    /// Back/select button.
    Back,
    /// Start/menu button.
    Start,
    /// Guide/system button.
    Guide,
    /// Left thumbstick press.
    LeftThumb,
    /// Right thumbstick press.
    RightThumb,
    /// D-pad up.
    DPadUp,
    /// D-pad right.
    DPadRight,
    /// D-pad down.
    DPadDown,
    /// D-pad left.
    DPadLeft,
    /// Last valid standard gamepad button.
    Last = DPadLeft
};

/// Standard gamepad axis code.
enum class GamepadAxis : uint8_t
{
    /// Left stick horizontal axis.
    LeftX,
    /// Left stick vertical axis.
    LeftY,
    /// Right stick horizontal axis.
    RightX,
    /// Right stick vertical axis.
    RightY,
    /// Left trigger axis.
    LeftTrigger,
    /// Right trigger axis.
    RightTrigger,
    /// Last valid standard gamepad axis.
    Last = RightTrigger
};

/// Number of standard gamepad button slots.
inline constexpr std::size_t GamepadButtonCount{ static_cast<std::size_t>(GamepadButton::Last) +
                                                 1 };
/// Number of standard gamepad axis slots.
inline constexpr std::size_t GamepadAxisCount{ static_cast<std::size_t>(GamepadAxis::Last) + 1 };
/// Maximum gamepad slots scanned by the backend.
inline constexpr std::size_t MaxGamepads{ 16 };
/// Maximum joystick slots scanned by the backend.
inline constexpr std::size_t MaxJoysticks{ 16 };

/// Connected gamepad metadata.
struct GamepadInfo
{
    /// Gamepad slot id.
    uint32_t id = 0;
    /// Human-readable device name when available.
    std::string name;
    /// Whether the device has a standard gamepad mapping.
    bool standardMapping = false;
};

/// Snapshot of a standard-mapped gamepad.
struct GamepadState
{
    /// Gamepad slot id.
    uint32_t id = 0;
    /// Human-readable device name when available.
    std::string name;
    /// Whether this state uses a standard mapping.
    bool standardMapping = false;
    /// Button states indexed by `GamepadButton`.
    std::array<bool, GamepadButtonCount> buttons{};
    /// Axis values indexed by `GamepadAxis`.
    std::array<float, GamepadAxisCount> axes{};

    /// Returns whether a standard gamepad button is down.
    [[nodiscard]] bool isButtonDown(GamepadButton button) const noexcept;
    /// Returns the current value for a standard gamepad axis.
    [[nodiscard]] float axis(GamepadAxis axis) const noexcept;
};

//----------------------------------------------------------------------------
//  Input State
//----------------------------------------------------------------------------
/// Main window type.
class Window;

namespace backend {
/// Backend-owned input snapshot storage.
struct InputStateData;
}  // namespace backend

/// Snapshot-style input query interface for a window.
class InputState final
{
    friend class Window;

public:
    /// Returns whether a key is currently down.
    bool isKeyDown(Key key) const;
    /// Returns whether a key transitioned from up to down this frame.
    bool isKeyPressed(Key key) const;
    /// Returns whether a key transitioned from down to up this frame.
    bool isKeyReleased(Key key) const;

    /// Returns whether a mouse button is currently down.
    bool isMouseButtonDown(MouseButton button) const;
    /// Returns whether a mouse button transitioned from up to down this frame.
    bool isMouseButtonPressed(MouseButton button) const;
    /// Returns whether a mouse button transitioned from down to up this frame.
    bool isMouseButtonReleased(MouseButton button) const;
    /// Returns the current cursor position in window coordinates.
    std::pair<double, double> mousePosition() const;
    /// Returns cursor movement since the previous poll/update.
    std::pair<double, double> mouseDelta() const;
    /// Returns accumulated scroll delta since the previous poll/update.
    std::pair<double, double> scrollDelta() const;
    /// Returns whether the cursor is inside the window content area.
    bool isMouseInside() const;

private:
    explicit InputState(const backend::InputStateData* state);

    const backend::InputStateData* state_;
};

//----------------------------------------------------------------------------
//  Input Helpers
//----------------------------------------------------------------------------
/// Keyboard key plus optional modifier requirements.
struct KeyBinding
{
    /// Keyboard key that triggers the action.
    Key key = Key::Unknown;
    /// Required modifiers. Empty modifiers mean plain key matching.
    Modifiers modifiers{};
    /// Additional keys that must be held, useful for side-specific modifiers.
    std::vector<Key> requiredKeys;
    /// When true, extra held modifiers prevent the binding from matching.
    bool exactModifiers = false;

    /// Compares two key bindings.
    [[nodiscard]] bool operator==(const KeyBinding&) const noexcept = default;
};

/// Direction filter for gamepad axis bindings.
enum class AxisDirection : int8_t
{
    /// Match either positive or negative axis movement.
    Any,
    /// Match only positive axis movement.
    Positive,
    /// Match only negative axis movement.
    Negative
};

/// Gamepad axis binding with a deadzone.
struct GamepadAxisBinding
{
    /// Standard gamepad axis that drives the action.
    GamepadAxis axis = GamepadAxis::LeftX;
    /// Absolute value below which the axis is ignored.
    float deadzone = 0.15f;
    /// Direction that activates the binding.
    AxisDirection direction = AxisDirection::Any;

    /// Compares two axis bindings.
    [[nodiscard]] bool operator==(const GamepadAxisBinding&) const noexcept = default;
};

/// Input sources that can trigger one named action.
struct ActionBinding
{
    /// Optional input context name. Empty means always enabled.
    std::string context;
    /// Keyboard keys that trigger the action, with optional modifiers.
    std::vector<KeyBinding> keys;
    /// Mouse buttons that trigger the action.
    std::vector<MouseButton> mouseButtons;
    /// Standard gamepad buttons that trigger the action.
    std::vector<GamepadButton> gamepadButtons;
    /// Standard gamepad axes that trigger or drive the action.
    std::vector<GamepadAxisBinding> gamepadAxes;
};

/// Stable handle for an action stored in an `ActionMap`.
struct ActionId
{
    /// Internal id. Zero means no action.
    uint32_t value = 0;

    /// Returns whether this handle refers to a possible action.
    [[nodiscard]] explicit operator bool() const noexcept
    {
        return value != 0;
    }

    /// Compares two action ids.
    [[nodiscard]] friend bool operator==(ActionId, ActionId) = default;
};

/// Optional action metadata for rebinding menus and debug UI.
struct ActionMetadata
{
    /// Human-readable label.
    std::string displayName;
    /// Longer UI/help description.
    std::string description;
};

/// Snapshot of one action and its current state.
struct ActionInfo
{
    /// Lightweight action id.
    ActionId id;
    /// Stable action name.
    std::string name;
    /// Optional UI metadata.
    ActionMetadata metadata;
    /// Current binding data.
    ActionBinding binding;
    /// Whether the action is currently down.
    bool down = false;
    /// Whether the action transitioned down on the last update.
    bool pressed = false;
    /// Whether the action transitioned up on the last update.
    bool released = false;
    /// Current axis value.
    float axisValue = 0.0f;
};

/// Small action binding map for game/app commands.
class ActionMap final
{
public:
    /// Returns an existing action id or creates one.
    [[nodiscard]] ActionId defineAction(std::string action);
    /// Returns an action id by name, or an empty id when missing.
    [[nodiscard]] ActionId findAction(std::string_view action) const noexcept;
    /// Returns whether an action exists.
    [[nodiscard]] bool hasAction(std::string_view action) const noexcept;
    /// Returns whether an action exists.
    [[nodiscard]] bool hasAction(ActionId action) const noexcept;
    /// Returns a snapshot of every action, suitable for debug UI or rebinding screens.
    [[nodiscard]] std::vector<ActionInfo> actions() const;

    /// Sets metadata for an existing action id.
    ActionMap& setMetadata(ActionId action, ActionMetadata metadata);
    /// Returns metadata for an action id, or null when missing.
    [[nodiscard]] const ActionMetadata* metadata(ActionId action) const noexcept;

    /// Binds a key plus optional required modifiers to an action id.
    ActionMap&
    bindKey(ActionId action, Key key, Modifiers modifiers = {}, bool exactModifiers = false);
    /// Binds a key plus additional required held keys to an action id.
    ActionMap& bindKeyCombo(ActionId action, Key key, std::vector<Key> requiredKeys);
    /// Binds a mouse button to an action id.
    ActionMap& bindMouseButton(ActionId action, MouseButton button);
    /// Binds a standard gamepad button to an action id.
    ActionMap& bindGamepadButton(ActionId action, GamepadButton button);
    /// Binds a standard gamepad axis to an action id.
    ActionMap& bindGamepadAxis(
        ActionId action,
        GamepadAxis axis,
        float deadzone = 0.15f,
        AxisDirection direction = AxisDirection::Any);
    /// Replaces key bindings for an action id.
    ActionMap&
    replaceKey(ActionId action, Key key, Modifiers modifiers = {}, bool exactModifiers = false);
    /// Replaces key bindings with a key plus additional required held keys for an action id.
    ActionMap& replaceKeyCombo(ActionId action, Key key, std::vector<Key> requiredKeys);
    /// Replaces mouse button bindings for an action id.
    ActionMap& replaceMouseButton(ActionId action, MouseButton button);
    /// Replaces gamepad button bindings for an action id.
    ActionMap& replaceGamepadButton(ActionId action, GamepadButton button);
    /// Replaces gamepad axis bindings for an action id.
    ActionMap& replaceGamepadAxis(
        ActionId action,
        GamepadAxis axis,
        float deadzone = 0.15f,
        AxisDirection direction = AxisDirection::Any);
    /// Removes all input bindings for an action id while preserving context and state.
    void clearBindings(ActionId action);
    /// Removes all bindings and state for one action id.
    void clear(ActionId action);
    /// Removes every binding and action state.
    void clearAll();
    /// Clears action transition state but keeps bindings.
    void resetState() noexcept;
    /// Assigns an action id to an input context. Empty context means always enabled.
    ActionMap& setContext(ActionId action, std::string context);
    /// Enables or disables an input context.
    void setContextEnabled(std::string context, bool enabled);
    /// Returns whether an input context is enabled. Unknown contexts are enabled.
    [[nodiscard]] bool isContextEnabled(std::string_view context) const noexcept;
    /// Clears all explicit context enable/disable state.
    void clearContextStates();

    /// Updates action states from input and an optional gamepad snapshot.
    template <typename Input>
    void update(const Input& input, const std::optional<GamepadState>& gamepad = std::nullopt)
    {
        for (auto& entry : entries_) {
            entry.previousDown = entry.down;

            if (!isContextEnabled(entry.binding.context)) {
                entry.down = false;
                entry.axisValue = 0.0f;
                continue;
            }

            entry.axisValue = bindingAxisValue(entry.binding, gamepad);
            entry.down =
                isDigitalBindingDown(entry.binding, input, gamepad) || entry.axisValue != 0.0f;
        }
    }

    /// Returns whether an action is currently down.
    [[nodiscard]] bool isDown(ActionId action) const;
    /// Returns whether an action id transitioned from up to down on the last update.
    [[nodiscard]] bool isPressed(ActionId action) const;
    /// Returns whether an action id transitioned from down to up on the last update.
    [[nodiscard]] bool isReleased(ActionId action) const;
    /// Returns the current axis value for an action id, or zero when inactive.
    [[nodiscard]] float axisValue(ActionId action) const;
    /// Returns the binding for an action id, or null if it does not exist.
    [[nodiscard]] const ActionBinding* binding(ActionId action) const noexcept;

private:
    struct ContextState
    {
        std::string context;
        bool enabled = true;
    };

    struct Entry
    {
        ActionId id;
        std::string action;
        ActionMetadata metadata;
        ActionBinding binding;
        bool down = false;
        bool previousDown = false;
        float axisValue = 0.0f;
    };

    [[nodiscard]] Entry* findEntry(std::string_view action) noexcept;
    [[nodiscard]] const Entry* findEntry(std::string_view action) const noexcept;
    [[nodiscard]] Entry* findEntry(ActionId action) noexcept;
    [[nodiscard]] const Entry* findEntry(ActionId action) const noexcept;
    [[nodiscard]] ContextState* findContext(std::string_view context) noexcept;
    [[nodiscard]] const ContextState* findContext(std::string_view context) const noexcept;
    [[nodiscard]] Entry& ensureEntry(std::string action);
    [[nodiscard]] ActionId nextActionId() noexcept;
    static void resetEntryState(Entry& entry) noexcept;

    template <typename Input>
    static bool isDigitalBindingDown(
        const ActionBinding& binding,
        const Input& input,
        const std::optional<GamepadState>& gamepad)
    {
        const Modifiers activeModifiers = currentModifiers(input);
        for (const KeyBinding& key : binding.keys) {
            if (input.isKeyDown(key.key) && requiredKeysDown(key.requiredKeys, input) &&
                modifiersMatch(key.modifiers, activeModifiers, key.exactModifiers)) {
                return true;
            }
        }

        for (MouseButton button : binding.mouseButtons) {
            if (input.isMouseButtonDown(button)) {
                return true;
            }
        }

        if (gamepad) {
            for (GamepadButton button : binding.gamepadButtons) {
                if (gamepad->isButtonDown(button)) {
                    return true;
                }
            }
        }

        return false;
    }

    template <typename Input>
    static bool requiredKeysDown(const std::vector<Key>& keys, const Input& input)
    {
        for (Key key : keys) {
            if (!input.isKeyDown(key)) {
                return false;
            }
        }

        return true;
    }

    template <typename Input>
    static Modifiers currentModifiers(const Input& input)
    {
        return Modifiers{
            .alt = input.isKeyDown(Key::LAlt) || input.isKeyDown(Key::RAlt),
            .control = input.isKeyDown(Key::LControl) || input.isKeyDown(Key::RControl),
            .shift = input.isKeyDown(Key::LShift) || input.isKeyDown(Key::RShift),
            .system = input.isKeyDown(Key::LSuper) || input.isKeyDown(Key::RSuper),
        };
    }

    static bool modifiersMatch(Modifiers required, Modifiers active, bool exact) noexcept
    {
        if (required.alt && !active.alt) {
            return false;
        }
        if (required.control && !active.control) {
            return false;
        }
        if (required.shift && !active.shift) {
            return false;
        }
        if (required.system && !active.system) {
            return false;
        }

        return !exact || required == active;
    }

    static float bindingAxisValue(
        const ActionBinding& binding,
        const std::optional<GamepadState>& gamepad) noexcept
    {
        if (!gamepad) {
            return 0.0f;
        }

        float result = 0.0f;
        for (const GamepadAxisBinding& axis : binding.gamepadAxes) {
            const float value = axisBindingValue(*gamepad, axis);
            if (std::abs(value) > std::abs(result)) {
                result = value;
            }
        }
        return result;
    }

    static float axisBindingValue(
        const GamepadState& gamepad,
        const GamepadAxisBinding& binding) noexcept
    {
        const float value = gamepad.axis(binding.axis);
        const float magnitude = std::abs(value);
        if (magnitude <= binding.deadzone) {
            return 0.0f;
        }

        switch (binding.direction) {
            case AxisDirection::Any:
                return value;
            case AxisDirection::Positive:
                return value > binding.deadzone ? value : 0.0f;
            case AxisDirection::Negative:
                return value < -binding.deadzone ? value : 0.0f;
        }

        return 0.0f;
    }

    std::vector<Entry> entries_;
    std::vector<ContextState> contexts_;
    uint32_t nextActionId_ = 1;
};

}  // namespace cwin

#endif
