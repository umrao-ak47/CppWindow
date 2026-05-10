/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#include "glfw_window.hpp"

#if defined(CPPWINDOW_PLATFORM_WINDOWS)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(CPPWINDOW_PLATFORM_MACOS)
#define GLFW_EXPOSE_NATIVE_COCOA
#elif defined(CPPWINDOW_PLATFORM_LINUX)
#if defined(CPPWINDOW_GLFW_HAS_X11)
#define GLFW_EXPOSE_NATIVE_X11
#endif
#if defined(CPPWINDOW_GLFW_HAS_WAYLAND)
#define GLFW_EXPOSE_NATIVE_WAYLAND
#endif
#else
#error "Unknown platform"
#endif
#include <GLFW/glfw3native.h>

namespace cwin::backend::glfw {

NativeHandles GLFWNativeWindow::nativeHandles() const
{
    NativeHandles handles{};

#if defined(CPPWINDOW_PLATFORM_WINDOWS)
    handles.system = NativeHandles::System::Win32;
    handles.window = glfwGetWin32Window(handle_.get());
#elif defined(CPPWINDOW_PLATFORM_MACOS)
    handles.system = NativeHandles::System::Cocoa;
    handles.window = glfwGetCocoaWindow(handle_.get());
#elif defined(CPPWINDOW_PLATFORM_LINUX)
#if defined(CPPWINDOW_GLFW_HAS_X11) || defined(CPPWINDOW_GLFW_HAS_WAYLAND)
    int platform = glfwGetPlatform();
#endif
#if defined(CPPWINDOW_GLFW_HAS_X11)
    if (platform == GLFW_PLATFORM_X11) {
        handles.system = NativeHandles::System::X11;
        handles.window = reinterpret_cast<void*>(glfwGetX11Window(handle_.get()));
        handles.display = glfwGetX11Display();
    }
#endif
#if defined(CPPWINDOW_GLFW_HAS_WAYLAND)
    if (platform == GLFW_PLATFORM_WAYLAND) {
        handles.system = NativeHandles::System::Wayland;
        handles.window = glfwGetWaylandWindow(handle_.get());
        handles.display = glfwGetWaylandDisplay();
    }
#endif
#endif

    return handles;
}

}  // namespace cwin::backend::glfw
