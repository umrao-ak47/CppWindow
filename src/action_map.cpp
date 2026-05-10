/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#include <cppwindow/cppwindow.hpp>

#include <algorithm>
#include <cmath>
#include <string_view>
#include <utility>
#include <vector>

namespace cwin {

//----------------------------------------------------------------------------
//  Action Map Implementation
//----------------------------------------------------------------------------
namespace {

template <typename T>
void appendUnique(std::vector<T>& values, T value)
{
    for (T existing : values) {
        if (existing == value) {
            return;
        }
    }

    values.push_back(value);
}

float sanitizeDeadzone(float deadzone) noexcept
{
    if (!std::isfinite(deadzone) || deadzone < 0.0f) {
        return 0.0f;
    }
    return std::clamp(deadzone, 0.0f, 1.0f);
}

}  // namespace

ActionId ActionMap::defineAction(std::string action)
{
    return ensureEntry(std::move(action)).id;
}

ActionId ActionMap::findAction(std::string_view action) const noexcept
{
    const Entry* entry = findEntry(action);
    return entry ? entry->id : ActionId{};
}

bool ActionMap::hasAction(std::string_view action) const noexcept
{
    return findEntry(action) != nullptr;
}

bool ActionMap::hasAction(ActionId action) const noexcept
{
    return findEntry(action) != nullptr;
}

std::vector<ActionInfo> ActionMap::actions() const
{
    std::vector<ActionInfo> actions;
    actions.reserve(entries_.size());

    for (const Entry& entry : entries_) {
        actions.push_back(
            ActionInfo{
                .id = entry.id,
                .name = entry.action,
                .metadata = entry.metadata,
                .binding = entry.binding,
                .down = entry.down,
                .pressed = entry.down && !entry.previousDown,
                .released = !entry.down && entry.previousDown,
                .axisValue = entry.axisValue,
            });
    }

    return actions;
}

ActionMap& ActionMap::setMetadata(ActionId action, ActionMetadata metadata)
{
    if (Entry* entry = findEntry(action)) {
        entry->metadata = std::move(metadata);
    }
    return *this;
}

const ActionMetadata* ActionMap::metadata(ActionId action) const noexcept
{
    const Entry* entry = findEntry(action);
    return entry ? &entry->metadata : nullptr;
}

ActionMap& ActionMap::bindKey(ActionId action, Key key, Modifiers modifiers, bool exactModifiers)
{
    if (Entry* entry = findEntry(action)) {
        appendUnique(
            entry->binding.keys,
            KeyBinding{
                .key = key,
                .modifiers = modifiers,
                .exactModifiers = exactModifiers,
            });
    }
    return *this;
}

ActionMap& ActionMap::bindKeyCombo(ActionId action, Key key, std::vector<Key> requiredKeys)
{
    if (Entry* entry = findEntry(action)) {
        appendUnique(
            entry->binding.keys,
            KeyBinding{
                .key = key,
                .requiredKeys = std::move(requiredKeys),
            });
    }
    return *this;
}

ActionMap& ActionMap::bindMouseButton(ActionId action, MouseButton button)
{
    if (Entry* entry = findEntry(action)) {
        appendUnique(entry->binding.mouseButtons, button);
    }
    return *this;
}

ActionMap& ActionMap::bindGamepadButton(ActionId action, GamepadButton button)
{
    if (Entry* entry = findEntry(action)) {
        appendUnique(entry->binding.gamepadButtons, button);
    }
    return *this;
}

ActionMap& ActionMap::bindGamepadAxis(
    ActionId action,
    GamepadAxis axis,
    float deadzone,
    AxisDirection direction)
{
    if (Entry* entry = findEntry(action)) {
        appendUnique(
            entry->binding.gamepadAxes,
            GamepadAxisBinding{
                .axis = axis,
                .deadzone = sanitizeDeadzone(deadzone),
                .direction = direction,
            });
    }
    return *this;
}

ActionMap& ActionMap::replaceKey(ActionId action, Key key, Modifiers modifiers, bool exactModifiers)
{
    if (Entry* entry = findEntry(action)) {
        entry->binding.keys.clear();
        appendUnique(
            entry->binding.keys,
            KeyBinding{
                .key = key,
                .modifiers = modifiers,
                .exactModifiers = exactModifiers,
            });
        resetEntryState(*entry);
    }
    return *this;
}

ActionMap& ActionMap::replaceKeyCombo(ActionId action, Key key, std::vector<Key> requiredKeys)
{
    if (Entry* entry = findEntry(action)) {
        entry->binding.keys.clear();
        appendUnique(
            entry->binding.keys,
            KeyBinding{
                .key = key,
                .requiredKeys = std::move(requiredKeys),
            });
        resetEntryState(*entry);
    }
    return *this;
}

ActionMap& ActionMap::replaceMouseButton(ActionId action, MouseButton button)
{
    if (Entry* entry = findEntry(action)) {
        entry->binding.mouseButtons.clear();
        appendUnique(entry->binding.mouseButtons, button);
        resetEntryState(*entry);
    }
    return *this;
}

ActionMap& ActionMap::replaceGamepadButton(ActionId action, GamepadButton button)
{
    if (Entry* entry = findEntry(action)) {
        entry->binding.gamepadButtons.clear();
        appendUnique(entry->binding.gamepadButtons, button);
        resetEntryState(*entry);
    }
    return *this;
}

ActionMap& ActionMap::replaceGamepadAxis(
    ActionId action,
    GamepadAxis axis,
    float deadzone,
    AxisDirection direction)
{
    if (Entry* entry = findEntry(action)) {
        entry->binding.gamepadAxes.clear();
        appendUnique(
            entry->binding.gamepadAxes,
            GamepadAxisBinding{
                .axis = axis,
                .deadzone = sanitizeDeadzone(deadzone),
                .direction = direction,
            });
        resetEntryState(*entry);
    }
    return *this;
}

void ActionMap::clearBindings(ActionId action)
{
    if (Entry* entry = findEntry(action)) {
        entry->binding.keys.clear();
        entry->binding.mouseButtons.clear();
        entry->binding.gamepadButtons.clear();
        entry->binding.gamepadAxes.clear();
        resetEntryState(*entry);
    }
}

void ActionMap::clear(ActionId action)
{
    if (!action) {
        return;
    }

    for (auto it = entries_.begin(); it != entries_.end(); ++it) {
        if (it->id == action) {
            entries_.erase(it);
            return;
        }
    }
}

void ActionMap::clearAll()
{
    entries_.clear();
    contexts_.clear();
}

void ActionMap::resetState() noexcept
{
    for (auto& entry : entries_) {
        resetEntryState(entry);
    }
}

ActionMap& ActionMap::setContext(ActionId action, std::string context)
{
    if (Entry* entry = findEntry(action)) {
        entry->binding.context = std::move(context);
        resetEntryState(*entry);
    }
    return *this;
}

void ActionMap::setContextEnabled(std::string context, bool enabled)
{
    if (context.empty()) {
        return;
    }

    if (ContextState* state = findContext(context)) {
        state->enabled = enabled;
        return;
    }

    contexts_.push_back(
        ContextState{
            .context = std::move(context),
            .enabled = enabled,
        });
}

bool ActionMap::isContextEnabled(std::string_view context) const noexcept
{
    if (context.empty()) {
        return true;
    }

    const ContextState* state = findContext(context);
    return !state || state->enabled;
}

void ActionMap::clearContextStates()
{
    contexts_.clear();
}

bool ActionMap::isDown(ActionId action) const noexcept
{
    const Entry* entry = findEntry(action);
    return entry && entry->down;
}

bool ActionMap::isPressed(ActionId action) const noexcept
{
    const Entry* entry = findEntry(action);
    return entry && entry->down && !entry->previousDown;
}

bool ActionMap::isReleased(ActionId action) const noexcept
{
    const Entry* entry = findEntry(action);
    return entry && !entry->down && entry->previousDown;
}

float ActionMap::axisValue(ActionId action) const noexcept
{
    const Entry* entry = findEntry(action);
    return entry ? entry->axisValue : 0.0f;
}

const ActionBinding* ActionMap::binding(ActionId action) const noexcept
{
    const Entry* entry = findEntry(action);
    return entry ? &entry->binding : nullptr;
}

ActionMap::Entry* ActionMap::findEntry(std::string_view action) noexcept
{
    for (auto& entry : entries_) {
        if (std::string_view{ entry.action } == action) {
            return &entry;
        }
    }

    return nullptr;
}

const ActionMap::Entry* ActionMap::findEntry(std::string_view action) const noexcept
{
    for (const auto& entry : entries_) {
        if (std::string_view{ entry.action } == action) {
            return &entry;
        }
    }

    return nullptr;
}

ActionMap::Entry* ActionMap::findEntry(ActionId action) noexcept
{
    if (!action) {
        return nullptr;
    }

    for (auto& entry : entries_) {
        if (entry.id == action) {
            return &entry;
        }
    }

    return nullptr;
}

const ActionMap::Entry* ActionMap::findEntry(ActionId action) const noexcept
{
    if (!action) {
        return nullptr;
    }

    for (const auto& entry : entries_) {
        if (entry.id == action) {
            return &entry;
        }
    }

    return nullptr;
}

ActionMap::ContextState* ActionMap::findContext(std::string_view context) noexcept
{
    for (auto& state : contexts_) {
        if (std::string_view{ state.context } == context) {
            return &state;
        }
    }

    return nullptr;
}

const ActionMap::ContextState* ActionMap::findContext(std::string_view context) const noexcept
{
    for (const auto& state : contexts_) {
        if (std::string_view{ state.context } == context) {
            return &state;
        }
    }

    return nullptr;
}

ActionMap::Entry& ActionMap::ensureEntry(std::string action)
{
    if (Entry* entry = findEntry(action)) {
        return *entry;
    }

    entries_.push_back(
        Entry{
            .id = nextActionId(),
            .action = std::move(action),
        });
    return entries_.back();
}

ActionId ActionMap::nextActionId() noexcept
{
    if (nextActionId_ == 0) {
        nextActionId_ = 1;
    }

    return ActionId{ nextActionId_++ };
}

void ActionMap::resetEntryState(Entry& entry) noexcept
{
    entry.down = false;
    entry.previousDown = false;
    entry.axisValue = 0.0f;
}

}  // namespace cwin
