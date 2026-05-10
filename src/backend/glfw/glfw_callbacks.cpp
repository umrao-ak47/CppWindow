/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#include "glfw_callbacks.hpp"

#include <algorithm>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "glfw_input_map.hpp"
#include "glfw_monitor.hpp"
#include "glfw_window.hpp"

namespace cwin::backend::glfw {

namespace {

template <typename... Ts>
struct Overloaded : Ts...
{
    using Ts::operator()...;
};

}  // namespace

void setupGlfwWindowHints(const WindowDesc& desc)
{
    glfwDefaultWindowHints();
    const bool createVisible =
        desc.visible && !desc.position && !desc.opacity && desc.windowMode == WindowMode::Windowed;

    // Common Window hints
    glfwWindowHint(GLFW_RESIZABLE, desc.resizable);
    glfwWindowHint(GLFW_VISIBLE, createVisible ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_FOCUSED, desc.focused);
    glfwWindowHint(GLFW_DECORATED, desc.decorated);
    glfwWindowHint(GLFW_FLOATING, desc.floating);
    // special Window Hints
    const auto visitor = Overloaded{
        [](NoneGraphicsModeTag) {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        },
        [](OpenGLGraphicsModeTag mode) {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, mode.config.major);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, mode.config.minor);
            glfwWindowHint(
                GLFW_OPENGL_PROFILE,
                mode.config.coreProfile ? GLFW_OPENGL_CORE_PROFILE : GLFW_OPENGL_ANY_PROFILE);
        },
    };
    std::visit(visitor, desc.mode);
}

void registerGlfwCallbacks(GLFWwindow* const handle)
{
    glfwSetFramebufferSizeCallback(handle, [](GLFWwindow* win, int w, int h) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(win));
        uint32_t width = static_cast<uint32_t>(w);
        uint32_t height = static_cast<uint32_t>(h);
        if (width == 0 || height == 0) {
            return;
        }
        self->handleEvent(
            Event::FramebufferResized{
                .width = width,
                .height = height,
            });
    });

    glfwSetWindowCloseCallback(handle, [](GLFWwindow* window) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        self->handleEvent(Event::Closed{});
        glfwSetWindowShouldClose(window, GLFW_FALSE);
    });

    glfwSetWindowRefreshCallback(handle, [](GLFWwindow* window) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        self->handleEvent(Event::Refresh{});
    });

    glfwSetWindowFocusCallback(handle, [](GLFWwindow* window, int focused) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        if (focused == GLFW_TRUE) {
            self->handleEvent(Event::FocusGained{});
        } else {
            self->handleEvent(Event::FocusLost{});
        }
    });

    glfwSetWindowSizeCallback(handle, [](GLFWwindow* window, int width, int height) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        self->handleMonitorChanged(getWindowMonitorId(window));
        self->handleWindowedBoundsChanged();
        self->handleEvent(
            Event::Resized{
                .width = width,
                .height = height,
            });
    });

    glfwSetWindowPosCallback(handle, [](GLFWwindow* window, int x, int y) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        self->handleMonitorChanged(getWindowMonitorId(window));
        self->handleWindowedBoundsChanged();
        self->handleEvent(
            Event::Moved{
                .x = x,
                .y = y,
            });
    });

    glfwSetWindowIconifyCallback(handle, [](GLFWwindow* window, int iconified) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        if (iconified == GLFW_TRUE) {
            self->handleEvent(Event::Minimized{});
        } else {
            self->handleWindowedBoundsChanged();
            self->handleEvent(Event::Restored{});
        }
    });

    glfwSetWindowMaximizeCallback(handle, [](GLFWwindow* window, int maximized) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        if (maximized == GLFW_TRUE) {
            self->handleEvent(Event::Maximized{});
        } else {
            self->handleWindowedBoundsChanged();
            self->handleEvent(Event::Restored{});
        }
    });

    glfwSetWindowContentScaleCallback(handle, [](GLFWwindow* window, float xScale, float yScale) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        self->handleEvent(
            Event::ContentScaleChanged{
                .xScale = xScale,
                .yScale = yScale,
            });
    });

    glfwSetKeyCallback(handle, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        Key mappedKey = toKey(key);
        if (mappedKey == Key::Unknown) {
            return;
        }
        if (action == GLFW_PRESS) {
            self->handleEvent(
                Event::KeyPressed{
                    .key = mappedKey,
                    .scancode = scancode,
                    .modifiers = toModifiers(mods),
                });
        } else if (action == GLFW_RELEASE) {
            self->handleEvent(
                Event::KeyReleased{
                    .key = mappedKey,
                    .scancode = scancode,
                    .modifiers = toModifiers(mods),
                });
        }
    });

    glfwSetCharCallback(handle, [](GLFWwindow* window, unsigned int codepoint) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        self->handleEvent(
            Event::TextEntered{
                .unicode = static_cast<char32_t>(codepoint),
            });
    });

    glfwSetMouseButtonCallback(handle, [](GLFWwindow* window, int button, int action, int mods) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        MouseButton mappedButton = toMouseButton(button);
        if (mappedButton == MouseButton::Unknown) {
            return;
        }
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        if (action == GLFW_PRESS) {
            self->handleEvent(
                Event::MouseButtonPressed{
                    .button = mappedButton,
                    .posX = xpos,
                    .posY = ypos,
                    .modifiers = toModifiers(mods),
                });
        } else if (action == GLFW_RELEASE) {
            self->handleEvent(
                Event::MouseButtonReleased{
                    .button = mappedButton,
                    .posX = xpos,
                    .posY = ypos,
                    .modifiers = toModifiers(mods),
                });
        }
    });

    glfwSetScrollCallback(handle, [](GLFWwindow* window, double xoffset, double yoffset) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        self->handleEvent(
            Event::MouseWheelScrolled{
                .deltaX = xoffset,
                .deltaY = yoffset,
                .posX = xpos,
                .posY = ypos,
            });
    });

    glfwSetCursorPosCallback(handle, [](GLFWwindow* window, double xpos, double ypos) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        self->handleEvent(
            Event::MouseMoved{
                .posX = xpos,
                .posY = ypos,
            });
    });

    glfwSetCursorEnterCallback(handle, [](GLFWwindow* window, int entered) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        if (entered == GLFW_TRUE) {
            self->handleEvent(Event::MouseEntered{});
        } else {
            self->handleEvent(Event::MouseLeft{});
        }
    });

    glfwSetDropCallback(handle, [](GLFWwindow* window, int count, const char** paths) {
        auto* self = static_cast<GLFWNativeWindow*>(glfwGetWindowUserPointer(window));
        std::vector<std::string> droppedPaths;
        droppedPaths.reserve(static_cast<size_t>(std::max(count, 0)));

        for (int i = 0; i < count; ++i) {
            if (paths[i]) {
                droppedPaths.emplace_back(paths[i]);
            }
        }

        double xpos = 0.0;
        double ypos = 0.0;
        glfwGetCursorPos(window, &xpos, &ypos);
        self->handleEvent(
            Event::FilesDropped{
                .paths = std::move(droppedPaths),
                .posX = xpos,
                .posY = ypos,
            });
    });
}

}  // namespace cwin::backend::glfw
