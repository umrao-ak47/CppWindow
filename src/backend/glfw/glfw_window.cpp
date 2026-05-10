/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#include "glfw_window.hpp"

#include <algorithm>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "glfw_callbacks.hpp"
#include "glfw_cursor.hpp"
#include "glfw_error.hpp"
#include "glfw_monitor.hpp"
#include "glfw_registry.hpp"

// Manually forward declare the Vulkan types GLFW needs.
typedef struct VkInstance_T* VkInstance;
typedef uint64_t VkSurfaceKHR;
typedef int VkResult;

extern "C" VkResult glfwCreateWindowSurface(
    VkInstance instance,
    GLFWwindow* window,
    const void* allocator,
    VkSurfaceKHR* surface);

namespace cwin::backend::glfw {

GLFWNativeWindow::GLFWNativeWindow(WindowDesc desc)
{
    hasOpenGLContext_ = std::holds_alternative<OpenGLGraphicsModeTag>(desc.mode);
    decorated_ = desc.decorated;
    floating_ = desc.floating;
    windowedDecorated_ = decorated_;
    windowedFloating_ = floating_;

    setupGlfwWindowHints(desc);
    clearGlfwError();
    handle_.reset(glfwCreateWindow(
        desc.width,
        desc.height,
        desc.title.c_str(),
        nullptr,  // monitor (fullscreen later)
        nullptr   // share
        ));

    if (!handle_) {
        throw makeGlfwError(ErrorCode::WindowCreationFailed, "Failed to create GLFW window");
    }

    // issue: https://github.com/glfw/glfw/issues/2060
    if (!desc.decorated) {
        glfwSetWindowAttrib(handle_.get(), GLFW_DECORATED, GLFW_FALSE);
    };

    if (desc.position) {
        glfwSetWindowPos(handle_.get(), desc.position->first, desc.position->second);
    }

    storage_ = std::make_shared<WindowStorage>(handle_.get());
    captureWindowedBounds();
    currentMonitorId_ = getWindowMonitorId(handle_.get());

    // set data and register callbacks
    glfwSetWindowUserPointer(handle_.get(), this);
    registerGlfwCallbacks(handle_.get());

    if (desc.sizeLimits) {
        setSizeLimits(*desc.sizeLimits);
    }
    if (desc.aspectRatio) {
        setAspectRatio(*desc.aspectRatio);
    }
    if (desc.opacity) {
        setOpacity(*desc.opacity);
    }
    if (desc.cursorMode) {
        setCursorMode(*desc.cursorMode);
    }
    if (desc.rawMouseMotion) {
        setRawMouseMotion(*desc.rawMouseMotion);
    }
    if (desc.vSync) {
        setVSync(*desc.vSync);
    }
    if (desc.windowMode != WindowMode::Windowed) {
        setWindowMode(desc.windowMode, desc.monitorId, desc.videoMode);
    }
    if (desc.visible &&
        (desc.position || desc.opacity || desc.windowMode != WindowMode::Windowed)) {
        glfwShowWindow(handle_.get());
        if (desc.focused) {
            glfwFocusWindow(handle_.get());
        }
    }

    // register to registry
    registerWindowStorage(storage_);
}

void GLFWNativeWindow::captureWindowedBounds()
{
    if (windowMode_ != WindowMode::Windowed || !handle_ || isMinimized() || isMaximized()) {
        return;
    }

    glfwGetWindowPos(handle_.get(), &windowedX_, &windowedY_);
    glfwGetWindowSize(handle_.get(), &windowedWidth_, &windowedHeight_);
    windowedDecorated_ = decorated_;
    windowedFloating_ = floating_;
}

void GLFWNativeWindow::handleEvent(Event&& event)
{
    storage_->inputState.handleEvent(event);
    storage_->eventQueue.push_back(std::move(event));
}

void GLFWNativeWindow::handleMonitorChanged(uint32_t monitorId)
{
    if (monitorId == currentMonitorId_) {
        return;
    }

    currentMonitorId_ = monitorId;
    handleEvent(
        Event::MonitorChanged{
            .monitorId = currentMonitorId_,
            .mode = windowMode_,
        });
}

void GLFWNativeWindow::handleWindowedBoundsChanged()
{
    captureWindowedBounds();
}

VulkanHandle GLFWNativeWindow::createVulkanSurface(void* instance) const
{
    VkSurfaceKHR surface = 0;

    clearGlfwError();
    // 3. Call the function directly
    // The linker will find this inside the glfw3 library you are linking against
    VkResult result = glfwCreateWindowSurface(
        static_cast<VkInstance>(instance),
        handle_.get(),
        nullptr,
        &surface);
    // VK_SUCCESS is 0
    if (result != 0) {
        throw makeGlfwError(
            ErrorCode::VulkanSurfaceCreationFailed,
            "Failed to create Vulkan window surface",
            "VkResult " + std::to_string(result));
    }
    return static_cast<VulkanHandle>(surface);
}

void GLFWNativeWindow::makeContextCurrent()
{
    glfwMakeContextCurrent(handle_.get());
}

void GLFWNativeWindow::swapBuffers()
{
    glfwSwapBuffers(handle_.get());
}

bool GLFWNativeWindow::shouldClose() const noexcept
{
    return glfwWindowShouldClose(handle_.get()) == GLFW_TRUE;
}

void GLFWNativeWindow::requestClose() noexcept
{
    glfwSetWindowShouldClose(handle_.get(), GLFW_TRUE);
}

std::span<const Event> GLFWNativeWindow::events() const noexcept
{
    return std::span<const Event>{ storage_->eventQueue.data(), storage_->eventQueue.size() };
}

const InputStateData* GLFWNativeWindow::inputData() const noexcept
{
    return &storage_->inputState.data();
}

void GLFWNativeWindow::setTitle(const std::string& title)
{
    glfwSetWindowTitle(handle_.get(), title.c_str());
}

std::string GLFWNativeWindow::title() const
{
    const char* currentTitle = glfwGetWindowTitle(handle_.get());
    return currentTitle ? std::string{ currentTitle } : std::string{};
}

void GLFWNativeWindow::setSize(int width, int height)
{
    glfwSetWindowSize(handle_.get(), width, height);
    captureWindowedBounds();
}

void GLFWNativeWindow::setPosition(int x, int y)
{
    glfwSetWindowPos(handle_.get(), x, y);
    captureWindowedBounds();
}

void GLFWNativeWindow::setSizeLimits(const SizeLimits& limits)
{
    glfwSetWindowSizeLimits(
        handle_.get(),
        limits.minWidth.value_or(GLFW_DONT_CARE),
        limits.minHeight.value_or(GLFW_DONT_CARE),
        limits.maxWidth.value_or(GLFW_DONT_CARE),
        limits.maxHeight.value_or(GLFW_DONT_CARE));
}

void GLFWNativeWindow::clearSizeLimits()
{
    glfwSetWindowSizeLimits(
        handle_.get(),
        GLFW_DONT_CARE,
        GLFW_DONT_CARE,
        GLFW_DONT_CARE,
        GLFW_DONT_CARE);
}

void GLFWNativeWindow::setAspectRatio(AspectRatio ratio)
{
    if (ratio.numerator <= 0 || ratio.denominator <= 0) {
        clearAspectRatio();
        return;
    }

    glfwSetWindowAspectRatio(handle_.get(), ratio.numerator, ratio.denominator);
}

void GLFWNativeWindow::clearAspectRatio()
{
    glfwSetWindowAspectRatio(handle_.get(), GLFW_DONT_CARE, GLFW_DONT_CARE);
}

void GLFWNativeWindow::setResizable(bool resizable)
{
    glfwSetWindowAttrib(handle_.get(), GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
}

void GLFWNativeWindow::setDecorated(bool decorated)
{
    glfwSetWindowAttrib(handle_.get(), GLFW_DECORATED, decorated ? GLFW_TRUE : GLFW_FALSE);
    decorated_ = decorated;
    if (windowMode_ == WindowMode::Windowed) {
        windowedDecorated_ = decorated_;
    }
}

void GLFWNativeWindow::setFloating(bool floating)
{
    glfwSetWindowAttrib(handle_.get(), GLFW_FLOATING, floating ? GLFW_TRUE : GLFW_FALSE);
    floating_ = floating;
    if (windowMode_ == WindowMode::Windowed) {
        windowedFloating_ = floating_;
    }
}

void GLFWNativeWindow::setOpacity(float opacity)
{
    glfwSetWindowOpacity(handle_.get(), std::clamp(opacity, 0.0f, 1.0f));
}

void GLFWNativeWindow::setVSync(bool enabled)
{
    if (!hasOpenGLContext_) {
        return;
    }

    GLFWwindow* previous = glfwGetCurrentContext();
    glfwMakeContextCurrent(handle_.get());
    glfwSwapInterval(enabled ? 1 : 0);
    glfwMakeContextCurrent(previous);
}

void GLFWNativeWindow::setCursorMode(CursorMode mode)
{
    glfwSetInputMode(handle_.get(), GLFW_CURSOR, toGlfwCursorMode(mode));
    cursorMode_ = mode;
}

bool GLFWNativeWindow::setCursorShape(CursorShape shape)
{
    UniqueGLFWcursor cursor(glfwCreateStandardCursor(toGlfwCursorShape(shape)));
    if (!cursor) {
        return false;
    }

    glfwSetCursor(handle_.get(), cursor.get());
    cursor_ = std::move(cursor);
    return true;
}

bool GLFWNativeWindow::setCursorImage(const ImageRgba& image, int hotX, int hotY)
{
    std::optional<GLFWimage> glfwImage = toGlfwImage(image);
    if (!glfwImage) {
        return false;
    }

    if (hotX < 0 || hotY < 0 || hotX >= glfwImage->width || hotY >= glfwImage->height) {
        return false;
    }

    UniqueGLFWcursor cursor(glfwCreateCursor(&*glfwImage, hotX, hotY));
    if (!cursor) {
        return false;
    }

    glfwSetCursor(handle_.get(), cursor.get());
    cursor_ = std::move(cursor);
    return true;
}

void GLFWNativeWindow::clearCursor()
{
    glfwSetCursor(handle_.get(), nullptr);
    cursor_.reset();
}

void GLFWNativeWindow::setMousePosition(double x, double y)
{
    glfwSetCursorPos(handle_.get(), x, y);
    storage_->inputState.setMousePosition(x, y);
}

bool GLFWNativeWindow::setRawMouseMotion(bool enabled)
{
    if (enabled && glfwRawMouseMotionSupported() != GLFW_TRUE) {
        return false;
    }

    glfwSetInputMode(handle_.get(), GLFW_RAW_MOUSE_MOTION, enabled ? GLFW_TRUE : GLFW_FALSE);
    return !enabled || isRawMouseMotionEnabled();
}

void GLFWNativeWindow::minimize()
{
    glfwIconifyWindow(handle_.get());
}

void GLFWNativeWindow::maximize()
{
    if (windowMode_ != WindowMode::Windowed) {
        setWindowMode(WindowMode::Windowed, currentMonitorId_, std::nullopt);
    }

    captureWindowedBounds();
    glfwMaximizeWindow(handle_.get());
}

void GLFWNativeWindow::restore()
{
    if (windowMode_ != WindowMode::Windowed) {
        setWindowMode(WindowMode::Windowed, currentMonitorId_, std::nullopt);
    }

    glfwRestoreWindow(handle_.get());
}

void GLFWNativeWindow::setWindowMode(
    WindowMode mode,
    uint32_t monitorId,
    std::optional<VideoMode> videoMode)
{
    if (mode == WindowMode::Windowed) {
        if (windowMode_ == WindowMode::Windowed) {
            return;
        }

        const bool wasExclusiveFullscreen = glfwGetWindowMonitor(handle_.get()) != nullptr;
        windowMode_ = WindowMode::Windowed;
        setDecorated(windowedDecorated_);
        setFloating(windowedFloating_);
        glfwRestoreWindow(handle_.get());

        if (wasExclusiveFullscreen) {
            glfwSetWindowMonitor(
                handle_.get(),
                nullptr,
                windowedX_,
                windowedY_,
                windowedWidth_,
                windowedHeight_,
                GLFW_DONT_CARE);
        } else {
            glfwSetWindowPos(handle_.get(), windowedX_, windowedY_);
            glfwSetWindowSize(handle_.get(), windowedWidth_, windowedHeight_);
        }

        return;
    }

    if (mode == windowMode_) {
        return;
    }

    GLFWmonitor* monitor = getMonitorById(monitorId);
    if (!monitor) {
        monitor = glfwGetPrimaryMonitor();
        monitorId = 0;
    }
    if (!monitor) {
        return;
    }

    if (mode == WindowMode::Fullscreen) {
        captureWindowedBounds();

        if (glfwGetWindowMonitor(handle_.get())) {
            int x = 0;
            int y = 0;
            int width = 0;
            int height = 0;
            glfwGetMonitorWorkarea(monitor, &x, &y, &width, &height);
            if (width <= 0 || height <= 0) {
                const GLFWvidmode* currentMode = glfwGetVideoMode(monitor);
                if (!currentMode) {
                    return;
                }

                glfwGetMonitorPos(monitor, &x, &y);
                width = currentMode->width;
                height = currentMode->height;
            }
            glfwSetWindowMonitor(handle_.get(), nullptr, x, y, width, height, GLFW_DONT_CARE);
        }

        windowMode_ = mode;
        setDecorated(true);
        setFloating(false);
        glfwMaximizeWindow(handle_.get());
        return;
    }

    if (mode == WindowMode::BorderlessFullscreen) {
        captureWindowedBounds();

        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
        glfwGetMonitorWorkarea(monitor, &x, &y, &width, &height);

        if (width <= 0 || height <= 0) {
            const GLFWvidmode* currentMode = glfwGetVideoMode(monitor);
            if (!currentMode) {
                return;
            }

            glfwGetMonitorPos(monitor, &x, &y);
            width = currentMode->width;
            height = currentMode->height;
        }

        windowMode_ = mode;
        setDecorated(false);
        setFloating(true);

        if (glfwGetWindowMonitor(handle_.get())) {
            glfwSetWindowMonitor(handle_.get(), nullptr, x, y, width, height, GLFW_DONT_CARE);
        } else {
            glfwSetWindowPos(handle_.get(), x, y);
            glfwSetWindowSize(handle_.get(), width, height);
        }

        return;
    }

    captureWindowedBounds();

    if (mode != WindowMode::ExclusiveFullscreen) {
        return;
    }

    int x = 0;
    int y = 0;
    glfwGetMonitorPos(monitor, &x, &y);

    VideoMode targetMode{};
    if (videoMode) {
        targetMode = *videoMode;
    } else if (const GLFWvidmode* currentMode = glfwGetVideoMode(monitor)) {
        targetMode = toVideoMode(*currentMode);
    }
    if (targetMode.width <= 0 || targetMode.height <= 0) {
        return;
    }

    windowMode_ = mode;
    glfwSetWindowMonitor(
        handle_.get(),
        monitor,
        x,
        y,
        targetMode.width,
        targetMode.height,
        targetMode.refreshRate);
}

bool GLFWNativeWindow::setIcon(std::span<const ImageRgba> images)
{
    if (images.empty()) {
        clearIcon();
        return true;
    }

    std::vector<GLFWimage> glfwImages;
    glfwImages.reserve(images.size());
    for (const ImageRgba& image : images) {
        std::optional<GLFWimage> glfwImage = toGlfwImage(image);
        if (!glfwImage) {
            return false;
        }
        glfwImages.push_back(*glfwImage);
    }

    clearGlfwError();
    glfwSetWindowIcon(handle_.get(), static_cast<int>(glfwImages.size()), glfwImages.data());
    return takeGlfwError().code == GLFW_NO_ERROR;
}

void GLFWNativeWindow::clearIcon()
{
    glfwSetWindowIcon(handle_.get(), 0, nullptr);
}

void GLFWNativeWindow::requestAttention()
{
    glfwRequestWindowAttention(handle_.get());
}

void GLFWNativeWindow::setFocus(bool focus) const noexcept
{
    if (focus) {
        glfwFocusWindow(handle_.get());
    }
}

void GLFWNativeWindow::setVisible(bool visible) const noexcept
{
    if (visible) {
        glfwShowWindow(handle_.get());
    } else {
        glfwHideWindow(handle_.get());
    }
}

WindowPlacement GLFWNativeWindow::windowedPlacement() const noexcept
{
    WindowPlacement placement{
        .x = windowedX_,
        .y = windowedY_,
        .width = windowedWidth_,
        .height = windowedHeight_,
        .maximized = isMaximized(),
    };

    if (windowMode_ == WindowMode::Windowed && !isMinimized() && !isMaximized()) {
        auto [x, y] = position();
        auto [width, height] = size();
        if (width > 0 && height > 0) {
            placement = WindowPlacement{
                .x = x,
                .y = y,
                .width = width,
                .height = height,
                .maximized = false,
            };
        }
    }

    return placement;
}

void GLFWNativeWindow::setWindowedPlacement(const WindowPlacement& placement)
{
    if (placement.width <= 0 || placement.height <= 0) {
        return;
    }

    if (windowMode_ != WindowMode::Windowed) {
        setWindowMode(WindowMode::Windowed, currentMonitorId_, std::nullopt);
    }

    glfwRestoreWindow(handle_.get());
    windowedX_ = placement.x;
    windowedY_ = placement.y;
    windowedWidth_ = placement.width;
    windowedHeight_ = placement.height;
    glfwSetWindowPos(handle_.get(), placement.x, placement.y);
    glfwSetWindowSize(handle_.get(), placement.width, placement.height);

    if (placement.maximized) {
        glfwMaximizeWindow(handle_.get());
    }
}

std::pair<int, int> GLFWNativeWindow::size() const noexcept
{
    int width, height;
    glfwGetWindowSize(handle_.get(), &width, &height);
    return {
        width,
        height,
    };
}

std::pair<int, int> GLFWNativeWindow::position() const noexcept
{
    int x, y;
    glfwGetWindowPos(handle_.get(), &x, &y);
    return {
        x,
        y,
    };
}

std::pair<uint32_t, uint32_t> GLFWNativeWindow::framebufferSize() const noexcept
{
    int width, height;
    glfwGetFramebufferSize(handle_.get(), &width, &height);
    return {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height),
    };
}

std::pair<float, float> GLFWNativeWindow::contentScale() const noexcept
{
    float xScale = 1.0f;
    float yScale = 1.0f;
    glfwGetWindowContentScale(handle_.get(), &xScale, &yScale);
    return {
        xScale,
        yScale,
    };
}

float GLFWNativeWindow::opacity() const noexcept
{
    return glfwGetWindowOpacity(handle_.get());
}

CursorMode GLFWNativeWindow::cursorMode() const noexcept
{
    return cursorMode_;
}

bool GLFWNativeWindow::isRawMouseMotionEnabled() const noexcept
{
    return glfwGetInputMode(handle_.get(), GLFW_RAW_MOUSE_MOTION) == GLFW_TRUE;
}

WindowMode GLFWNativeWindow::windowMode() const noexcept
{
    return windowMode_;
}

bool GLFWNativeWindow::isResizable() const noexcept
{
    return glfwGetWindowAttrib(handle_.get(), GLFW_RESIZABLE) == GLFW_TRUE;
}

bool GLFWNativeWindow::isDecorated() const noexcept
{
    return glfwGetWindowAttrib(handle_.get(), GLFW_DECORATED) == GLFW_TRUE;
}

bool GLFWNativeWindow::isFloating() const noexcept
{
    return glfwGetWindowAttrib(handle_.get(), GLFW_FLOATING) == GLFW_TRUE;
}

bool GLFWNativeWindow::isMinimized() const noexcept
{
    return glfwGetWindowAttrib(handle_.get(), GLFW_ICONIFIED) == GLFW_TRUE;
}

bool GLFWNativeWindow::isMaximized() const noexcept
{
    return glfwGetWindowAttrib(handle_.get(), GLFW_MAXIMIZED) == GLFW_TRUE;
}

bool GLFWNativeWindow::isFocused() const noexcept
{
    return glfwGetWindowAttrib(handle_.get(), GLFW_FOCUSED) == GLFW_TRUE;
}

bool GLFWNativeWindow::isVisible() const noexcept
{
    return glfwGetWindowAttrib(handle_.get(), GLFW_VISIBLE) == GLFW_TRUE;
}

}  // namespace cwin::backend::glfw
