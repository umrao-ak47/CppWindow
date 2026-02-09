/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#pragma once

#include <glfw-vk/Events.hpp>
#include <glfw-vk/Input.hpp>

#include <functional>
#include <optional>
#include <string>

namespace glfwvk {

struct WindowDesc
{
    std::string title = "glfw-vk window";
    uint32_t width = 1280;
    uint32_t height = 720;
    bool resizable = true;
    bool visible = true;
    bool decorated = true;
    bool focused = true;
};

class Window
{
public:
    // We use a typedef to make it clear what this integer represents
    // In Vulkan, handles are 64-bit integers.
    using SurfaceHandle = uint64_t;

    explicit Window(const WindowDesc& desc);
    Window(Window&&) noexcept;
    Window(const Window&) = delete;
    ~Window();

    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) noexcept;

    void* nativeHandle() const noexcept;
    SurfaceHandle createSurface(void* instance) const;

    bool shouldClose() const noexcept;
    void requestClose() noexcept;

    void pollEvents() const noexcept;
    std::optional<Event> pollEvent() const noexcept;
    const Input& getInput() const noexcept;

    void setTitle(const std::string& title);
    void setSize(int width, int height);
    std::pair<int, int> getSize() const noexcept;
    std::pair<uint32_t, uint32_t> getFrameBufferSize() const noexcept;
    bool isFocused() const noexcept;
    bool isVisible() const noexcept;

private:
    void registerCallbacks();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace glfwvk
