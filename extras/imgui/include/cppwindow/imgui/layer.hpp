/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

/// @file layer.hpp
/// @brief Renderer-agnostic Dear ImGui frame helper.

#ifndef CPPWINDOW_HEADER_IMGUI_LAYER_HPP
#define CPPWINDOW_HEADER_IMGUI_LAYER_HPP

#include <cppwindow/imgui/platform.hpp>

#include <concepts>
#include <imgui.h>
#include <span>
#include <utility>

namespace cwin::imgui {

/// Renderer adapter requirements used by `Layer`.
template <typename T>
concept Renderer = requires(T renderer, ImDrawData* drawData) {
    { renderer.newFrame() } -> std::same_as<void>;
    { renderer.render(drawData) } -> std::same_as<void>;
};

/// Coordinates the common ImGui platform + renderer frame flow.
///
/// `RendererT` is supplied by the application. It can wrap an official Dear
/// ImGui renderer backend or a renderer owned by the app/game engine.
template <Renderer RendererT>
class Layer final
{
public:
    /// Creates the platform backend and constructs the renderer in place.
    template <typename... Args>
        requires std::constructible_from<RendererT, Args...>
    explicit Layer(Window& window, Args&&... args)
        : platform_(window),
          renderer_(std::forward<Args>(args)...)
    {
    }

    /// Creates the platform backend and takes ownership of an existing renderer.
    explicit Layer(Window& window, RendererT renderer)
        : platform_(window),
          renderer_(std::move(renderer))
    {
    }

    Layer(const Layer&) = delete;
    Layer& operator=(const Layer&) = delete;
    Layer(Layer&&) = delete;
    Layer& operator=(Layer&&) = delete;

    /// Forwards CppWindow events to Dear ImGui.
    void handleEvents(std::span<const Event> events)
    {
        platform_.handleEvents(events);
    }

    /// Starts a new ImGui frame using the renderer and platform backends.
    void newFrame()
    {
        renderer_.newFrame();
        platform_.newFrame();
        ImGui::NewFrame();
    }

    /// Finalizes ImGui and asks the renderer to draw the generated draw data.
    void render()
    {
        ImGui::Render();
        renderer_.render(ImGui::GetDrawData());
    }

    /// Returns the platform backend.
    [[nodiscard]] Platform& platform() noexcept
    {
        return platform_;
    }

    /// Returns the platform backend.
    [[nodiscard]] const Platform& platform() const noexcept
    {
        return platform_;
    }

    /// Returns the renderer adapter.
    [[nodiscard]] RendererT& renderer() noexcept
    {
        return renderer_;
    }

    /// Returns the renderer adapter.
    [[nodiscard]] const RendererT& renderer() const noexcept
    {
        return renderer_;
    }

    /// Returns whether ImGui wants mouse input for the current frame.
    [[nodiscard]] bool wantsMouse() const noexcept
    {
        return platform_.wantsMouse();
    }

    /// Returns whether ImGui wants keyboard input for the current frame.
    [[nodiscard]] bool wantsKeyboard() const noexcept
    {
        return platform_.wantsKeyboard();
    }

    /// Returns whether ImGui wants text input for the current frame.
    [[nodiscard]] bool wantsTextInput() const noexcept
    {
        return platform_.wantsTextInput();
    }

private:
    Platform platform_;
    RendererT renderer_;
};

}  // namespace cwin::imgui

#endif  // CPPWINDOW_HEADER_IMGUI_LAYER_HPP
