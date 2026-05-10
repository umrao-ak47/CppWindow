/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#include <cppwindow/imgui/platform.hpp>

#include <algorithm>
#include <cfloat>
#include <chrono>
#include <imgui.h>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace cwin::imgui {
namespace {

[[nodiscard]] ImGuiKey toImGuiKey(Key key) noexcept
{
    switch (key) {
        case Key::Space:
            return ImGuiKey_Space;
        case Key::Apostrophe:
            return ImGuiKey_Apostrophe;
        case Key::Comma:
            return ImGuiKey_Comma;
        case Key::Minus:
            return ImGuiKey_Minus;
        case Key::Period:
            return ImGuiKey_Period;
        case Key::Slash:
            return ImGuiKey_Slash;
        case Key::Num0:
            return ImGuiKey_0;
        case Key::Num1:
            return ImGuiKey_1;
        case Key::Num2:
            return ImGuiKey_2;
        case Key::Num3:
            return ImGuiKey_3;
        case Key::Num4:
            return ImGuiKey_4;
        case Key::Num5:
            return ImGuiKey_5;
        case Key::Num6:
            return ImGuiKey_6;
        case Key::Num7:
            return ImGuiKey_7;
        case Key::Num8:
            return ImGuiKey_8;
        case Key::Num9:
            return ImGuiKey_9;
        case Key::Semicolon:
            return ImGuiKey_Semicolon;
        case Key::Equal:
            return ImGuiKey_Equal;
        case Key::A:
            return ImGuiKey_A;
        case Key::B:
            return ImGuiKey_B;
        case Key::C:
            return ImGuiKey_C;
        case Key::D:
            return ImGuiKey_D;
        case Key::E:
            return ImGuiKey_E;
        case Key::F:
            return ImGuiKey_F;
        case Key::G:
            return ImGuiKey_G;
        case Key::H:
            return ImGuiKey_H;
        case Key::I:
            return ImGuiKey_I;
        case Key::J:
            return ImGuiKey_J;
        case Key::K:
            return ImGuiKey_K;
        case Key::L:
            return ImGuiKey_L;
        case Key::M:
            return ImGuiKey_M;
        case Key::N:
            return ImGuiKey_N;
        case Key::O:
            return ImGuiKey_O;
        case Key::P:
            return ImGuiKey_P;
        case Key::Q:
            return ImGuiKey_Q;
        case Key::R:
            return ImGuiKey_R;
        case Key::S:
            return ImGuiKey_S;
        case Key::T:
            return ImGuiKey_T;
        case Key::U:
            return ImGuiKey_U;
        case Key::V:
            return ImGuiKey_V;
        case Key::W:
            return ImGuiKey_W;
        case Key::X:
            return ImGuiKey_X;
        case Key::Y:
            return ImGuiKey_Y;
        case Key::Z:
            return ImGuiKey_Z;
        case Key::LBracket:
            return ImGuiKey_LeftBracket;
        case Key::Backslash:
            return ImGuiKey_Backslash;
        case Key::RBracket:
            return ImGuiKey_RightBracket;
        case Key::Grave:
            return ImGuiKey_GraveAccent;
        case Key::Escape:
            return ImGuiKey_Escape;
        case Key::Enter:
            return ImGuiKey_Enter;
        case Key::Tab:
            return ImGuiKey_Tab;
        case Key::Backspace:
            return ImGuiKey_Backspace;
        case Key::Insert:
            return ImGuiKey_Insert;
        case Key::Delete:
            return ImGuiKey_Delete;
        case Key::Right:
            return ImGuiKey_RightArrow;
        case Key::Left:
            return ImGuiKey_LeftArrow;
        case Key::Down:
            return ImGuiKey_DownArrow;
        case Key::Up:
            return ImGuiKey_UpArrow;
        case Key::PageUp:
            return ImGuiKey_PageUp;
        case Key::PageDown:
            return ImGuiKey_PageDown;
        case Key::Home:
            return ImGuiKey_Home;
        case Key::End:
            return ImGuiKey_End;
        case Key::CapsLock:
            return ImGuiKey_CapsLock;
        case Key::ScrollLock:
            return ImGuiKey_ScrollLock;
        case Key::NumLock:
            return ImGuiKey_NumLock;
        case Key::PrintScreen:
            return ImGuiKey_PrintScreen;
        case Key::Pause:
            return ImGuiKey_Pause;
        case Key::F1:
            return ImGuiKey_F1;
        case Key::F2:
            return ImGuiKey_F2;
        case Key::F3:
            return ImGuiKey_F3;
        case Key::F4:
            return ImGuiKey_F4;
        case Key::F5:
            return ImGuiKey_F5;
        case Key::F6:
            return ImGuiKey_F6;
        case Key::F7:
            return ImGuiKey_F7;
        case Key::F8:
            return ImGuiKey_F8;
        case Key::F9:
            return ImGuiKey_F9;
        case Key::F10:
            return ImGuiKey_F10;
        case Key::F11:
            return ImGuiKey_F11;
        case Key::F12:
            return ImGuiKey_F12;
        case Key::F13:
            return ImGuiKey_F13;
        case Key::F14:
            return ImGuiKey_F14;
        case Key::F15:
            return ImGuiKey_F15;
        case Key::F16:
            return ImGuiKey_F16;
        case Key::F17:
            return ImGuiKey_F17;
        case Key::F18:
            return ImGuiKey_F18;
        case Key::F19:
            return ImGuiKey_F19;
        case Key::F20:
            return ImGuiKey_F20;
        case Key::F21:
            return ImGuiKey_F21;
        case Key::F22:
            return ImGuiKey_F22;
        case Key::F23:
            return ImGuiKey_F23;
        case Key::F24:
            return ImGuiKey_F24;
        case Key::Numpad0:
            return ImGuiKey_Keypad0;
        case Key::Numpad1:
            return ImGuiKey_Keypad1;
        case Key::Numpad2:
            return ImGuiKey_Keypad2;
        case Key::Numpad3:
            return ImGuiKey_Keypad3;
        case Key::Numpad4:
            return ImGuiKey_Keypad4;
        case Key::Numpad5:
            return ImGuiKey_Keypad5;
        case Key::Numpad6:
            return ImGuiKey_Keypad6;
        case Key::Numpad7:
            return ImGuiKey_Keypad7;
        case Key::Numpad8:
            return ImGuiKey_Keypad8;
        case Key::Numpad9:
            return ImGuiKey_Keypad9;
        case Key::NumpadDecimal:
            return ImGuiKey_KeypadDecimal;
        case Key::NumpadDivide:
            return ImGuiKey_KeypadDivide;
        case Key::NumpadMultiply:
            return ImGuiKey_KeypadMultiply;
        case Key::NumpadSubtract:
            return ImGuiKey_KeypadSubtract;
        case Key::NumpadAdd:
            return ImGuiKey_KeypadAdd;
        case Key::NumpadEnter:
            return ImGuiKey_KeypadEnter;
        case Key::NumpadEqual:
            return ImGuiKey_KeypadEqual;
        case Key::LShift:
            return ImGuiKey_LeftShift;
        case Key::LControl:
            return ImGuiKey_LeftCtrl;
        case Key::LAlt:
            return ImGuiKey_LeftAlt;
        case Key::LSuper:
            return ImGuiKey_LeftSuper;
        case Key::RShift:
            return ImGuiKey_RightShift;
        case Key::RControl:
            return ImGuiKey_RightCtrl;
        case Key::RAlt:
            return ImGuiKey_RightAlt;
        case Key::RSuper:
            return ImGuiKey_RightSuper;
        case Key::Menu:
            return ImGuiKey_Menu;
        case Key::Unknown:
        case Key::F25:
        case Key::World1:
        case Key::World2:
            break;
    }

    return ImGuiKey_None;
}

[[nodiscard]] int toImGuiMouseButton(MouseButton button) noexcept
{
    switch (button) {
        case MouseButton::Left:
            return 0;
        case MouseButton::Right:
            return 1;
        case MouseButton::Middle:
            return 2;
        case MouseButton::Button4:
            return 3;
        case MouseButton::Button5:
            return 4;
        case MouseButton::Unknown:
        case MouseButton::Button6:
        case MouseButton::Button7:
        case MouseButton::Button8:
            break;
    }

    return -1;
}

void addModifierEvents(ImGuiIO& io, Modifiers modifiers) noexcept
{
    io.AddKeyEvent(ImGuiMod_Ctrl, modifiers.control);
    io.AddKeyEvent(ImGuiMod_Shift, modifiers.shift);
    io.AddKeyEvent(ImGuiMod_Alt, modifiers.alt);
    io.AddKeyEvent(ImGuiMod_Super, modifiers.system);
}

void clearModifierEvents(ImGuiIO& io) noexcept
{
    addModifierEvents(io, {});
}

[[nodiscard]] CursorShape toCursorShape(ImGuiMouseCursor cursor) noexcept
{
    switch (cursor) {
        case ImGuiMouseCursor_TextInput:
            return CursorShape::IBeam;
        case ImGuiMouseCursor_ResizeAll:
            return CursorShape::ResizeAll;
        case ImGuiMouseCursor_ResizeNS:
            return CursorShape::ResizeVertical;
        case ImGuiMouseCursor_ResizeEW:
            return CursorShape::ResizeHorizontal;
        case ImGuiMouseCursor_ResizeNESW:
            return CursorShape::ResizeDiagonalNESW;
        case ImGuiMouseCursor_ResizeNWSE:
            return CursorShape::ResizeDiagonalNWSE;
        case ImGuiMouseCursor_Hand:
            return CursorShape::Hand;
        case ImGuiMouseCursor_NotAllowed:
            return CursorShape::NotAllowed;
        case ImGuiMouseCursor_Arrow:
        case ImGuiMouseCursor_None:
        case ImGuiMouseCursor_COUNT:
            break;
    }

    return CursorShape::Arrow;
}

void handleEventPayload(ImGuiIO& io, const Event::TextEntered& text)
{
    if (text.unicode != U'\0') {
        io.AddInputCharacter(static_cast<unsigned int>(text.unicode));
    }
}

void handleEventPayload(ImGuiIO& io, const Event::KeyPressed& key)
{
    addModifierEvents(io, key.modifiers);
    const ImGuiKey imguiKey = toImGuiKey(key.key);
    if (imguiKey != ImGuiKey_None) {
        io.AddKeyEvent(imguiKey, true);
    }
}

void handleEventPayload(ImGuiIO& io, const Event::KeyReleased& key)
{
    addModifierEvents(io, key.modifiers);
    const ImGuiKey imguiKey = toImGuiKey(key.key);
    if (imguiKey != ImGuiKey_None) {
        io.AddKeyEvent(imguiKey, false);
    }
}

void handleEventPayload(ImGuiIO& io, const Event::MouseWheelScrolled& scroll)
{
    io.AddMousePosEvent(static_cast<float>(scroll.posX), static_cast<float>(scroll.posY));
    io.AddMouseWheelEvent(static_cast<float>(scroll.deltaX), static_cast<float>(scroll.deltaY));
}

void handleEventPayload(ImGuiIO& io, const Event::MouseButtonPressed& mouse)
{
    addModifierEvents(io, mouse.modifiers);
    io.AddMousePosEvent(static_cast<float>(mouse.posX), static_cast<float>(mouse.posY));
    const int button = toImGuiMouseButton(mouse.button);
    if (button >= 0) {
        io.AddMouseButtonEvent(button, true);
    }
}

void handleEventPayload(ImGuiIO& io, const Event::MouseButtonReleased& mouse)
{
    addModifierEvents(io, mouse.modifiers);
    io.AddMousePosEvent(static_cast<float>(mouse.posX), static_cast<float>(mouse.posY));
    const int button = toImGuiMouseButton(mouse.button);
    if (button >= 0) {
        io.AddMouseButtonEvent(button, false);
    }
}

void handleEventPayload(ImGuiIO& io, const Event::MouseMoved& mouse)
{
    io.AddMousePosEvent(static_cast<float>(mouse.posX), static_cast<float>(mouse.posY));
}

void handleEventPayload(ImGuiIO& io, const Event::MouseLeft&)
{
    io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
}

void handleEventPayload(ImGuiIO& io, const Event::FocusGained&)
{
    io.AddFocusEvent(true);
}

void handleEventPayload(ImGuiIO& io, const Event::FocusLost&)
{
    io.AddFocusEvent(false);
    clearModifierEvents(io);
}

template <typename T>
void handleEventPayload(ImGuiIO&, const T&)
{
}

}  // namespace

Platform::Platform(Window& window)
    : window_(&window)
{
    if (ImGui::GetCurrentContext() == nullptr) {
        throw std::runtime_error("cwin::imgui::Platform requires an active ImGui context");
    }

    ImGuiIO& io = ImGui::GetIO();
    if (io.BackendPlatformUserData != nullptr) {
        throw std::runtime_error("Dear ImGui already has a platform backend for this context");
    }

    io.BackendPlatformUserData = this;
    io.BackendPlatformName = "cppwindow";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    io.SetClipboardTextFn = &Platform::setClipboardText;
    io.GetClipboardTextFn = &Platform::clipboardText;
    io.ClipboardUserData = this;
}

Platform::~Platform()
{
    if (ImGui::GetCurrentContext() == nullptr) {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    if (io.BackendPlatformUserData != this) {
        return;
    }

    io.BackendPlatformName = nullptr;
    io.BackendPlatformUserData = nullptr;
    io.BackendFlags &= ~ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags &= ~ImGuiBackendFlags_HasSetMousePos;
    io.SetClipboardTextFn = nullptr;
    io.GetClipboardTextFn = nullptr;
    io.ClipboardUserData = nullptr;
}

void Platform::handleEvents(std::span<const Event> events)
{
    ImGuiIO& io = ImGui::GetIO();
    for (const Event& event : events) {
        event.visit([&io](const auto& payload) {
            handleEventPayload(io, payload);
        });
    }
}

void Platform::newFrame()
{
    ImGuiIO& io = ImGui::GetIO();

    auto [width, height] = window_->size();
    auto [framebufferWidth, framebufferHeight] = window_->framebufferSize();
    width = std::max(width, 0);
    height = std::max(height, 0);

    io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));
    io.DisplayFramebufferScale = ImVec2(
        width > 0 ? static_cast<float>(framebufferWidth) / static_cast<float>(width) : 1.0f,
        height > 0 ? static_cast<float>(framebufferHeight) / static_cast<float>(height) : 1.0f);

    const auto now = std::chrono::steady_clock::now();
    if (hasFrameTime_) {
        const auto delta = std::chrono::duration<float>(now - lastFrameTime_).count();
        io.DeltaTime = delta > 0.0f ? delta : 1.0f / 60.0f;
    } else {
        io.DeltaTime = 1.0f / 60.0f;
        hasFrameTime_ = true;
    }
    lastFrameTime_ = now;

    if (io.WantSetMousePos) {
        window_->setMousePosition(io.MousePos.x, io.MousePos.y);
    }

    updateMouseCursor();
}

bool Platform::wantsMouse() const noexcept
{
    return ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureMouse;
}

bool Platform::wantsKeyboard() const noexcept
{
    return ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureKeyboard;
}

bool Platform::wantsTextInput() const noexcept
{
    return ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantTextInput;
}

void Platform::setMouseCursorUpdatesEnabled(bool enabled) noexcept
{
    updateMouseCursor_ = enabled;
}

bool Platform::mouseCursorUpdatesEnabled() const noexcept
{
    return updateMouseCursor_;
}

const char* Platform::clipboardText(void* userData)
{
    auto* platform = static_cast<Platform*>(userData);
    platform->clipboardText_ = cwin::Context::get().clipboardText().value_or(std::string{});
    return platform->clipboardText_.c_str();
}

void Platform::setClipboardText(void*, const char* text)
{
    (void)cwin::Context::get().setClipboardText(
        text != nullptr ? std::string{ text } : std::string{});
}

void Platform::updateMouseCursor()
{
    if (!updateMouseCursor_) {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) != 0) {
        return;
    }

    const ImGuiMouseCursor cursor = ImGui::GetMouseCursor();
    if (io.MouseDrawCursor || cursor == ImGuiMouseCursor_None) {
        window_->setCursorMode(CursorMode::Hidden);
        return;
    }

    window_->setCursorMode(CursorMode::Normal);
    (void)window_->setCursorShape(toCursorShape(cursor));
}

}  // namespace cwin::imgui
