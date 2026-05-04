/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

/// @file context.hpp
/// @brief RAII Dear ImGui context helper.

#ifndef CPPWINDOW_HEADER_IMGUI_CONTEXT_HPP
#define CPPWINDOW_HEADER_IMGUI_CONTEXT_HPP

#include <imgui.h>

namespace cwin::imgui {

/// Built-in style to apply after creating a Dear ImGui context.
enum class Style
{
    /// Do not apply a style. The app configures styling explicitly.
    None,
    /// Apply Dear ImGui's dark style.
    Dark,
    /// Apply Dear ImGui's light style.
    Light,
    /// Apply Dear ImGui's classic style.
    Classic
};

/// Options used when creating a Dear ImGui context.
struct ContextOptions
{
    /// Optional built-in style to apply after context creation.
    Style style = Style::None;
};

/// Owns a Dear ImGui context for the current thread.
///
/// Construct this before `Platform` or `Layer`, and ensure it outlives them.
/// This helper only owns the Dear ImGui context; renderer/device resources
/// remain owned by the application renderer adapter.
class Context final
{
public:
    /// Creates a Dear ImGui context and optionally applies a built-in style.
    explicit Context(ContextOptions options = {})
    {
        IMGUI_CHECKVERSION();
        context_ = ImGui::CreateContext();
        ImGui::SetCurrentContext(context_);
        applyStyle(options.style);
    }

    /// Destroys the owned Dear ImGui context.
    ~Context()
    {
        if (context_) {
            ImGui::DestroyContext(context_);
        }
    }

    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(Context&&) = delete;

    /// Returns the owned Dear ImGui context.
    [[nodiscard]] ImGuiContext* get() const noexcept
    {
        return context_;
    }

    /// Makes the owned Dear ImGui context current.
    void makeCurrent() const noexcept
    {
        ImGui::SetCurrentContext(context_);
    }

private:
    static void applyStyle(Style style)
    {
        switch (style) {
            case Style::None:
                return;
            case Style::Dark:
                ImGui::StyleColorsDark();
                return;
            case Style::Light:
                ImGui::StyleColorsLight();
                return;
            case Style::Classic:
                ImGui::StyleColorsClassic();
                return;
        }
    }

    ImGuiContext* context_ = nullptr;
};

}  // namespace cwin::imgui

#endif  // CPPWINDOW_HEADER_IMGUI_CONTEXT_HPP
