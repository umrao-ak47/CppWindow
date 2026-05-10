/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#include "glfw_monitor.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

namespace cwin::backend::glfw {

std::vector<GLFWmonitor*> getOrderedMonitors()
{
    int count = 0;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    std::vector<GLFWmonitor*> ordered;
    if (!monitors || count <= 0) {
        return ordered;
    }

    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    if (primary) {
        ordered.push_back(primary);
    }

    for (int i = 0; i < count; ++i) {
        if (monitors[i] && monitors[i] != primary) {
            ordered.push_back(monitors[i]);
        }
    }

    return ordered;
}

GLFWmonitor* getMonitorById(uint32_t monitorId) noexcept
{
    int count = 0;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    if (!monitors || count <= 0) {
        return nullptr;
    }

    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    if (monitorId == 0) {
        return primary ? primary : monitors[0];
    }

    uint32_t currentId = primary ? 1 : 0;
    for (int i = 0; i < count; ++i) {
        if (!monitors[i] || monitors[i] == primary) {
            continue;
        }

        if (currentId == monitorId) {
            return monitors[i];
        }
        ++currentId;
    }

    return nullptr;
}

VideoMode toVideoMode(const GLFWvidmode& mode) noexcept
{
    return {
        .width = mode.width,
        .height = mode.height,
        .redBits = mode.redBits,
        .greenBits = mode.greenBits,
        .blueBits = mode.blueBits,
        .refreshRate = mode.refreshRate,
    };
}

MonitorInfo toMonitorInfo(GLFWmonitor* monitor, uint32_t id, bool primary)
{
    MonitorInfo info{};
    info.id = id;
    info.primary = primary;

    if (const char* name = glfwGetMonitorName(monitor)) {
        info.name = name;
    }

    glfwGetMonitorPos(monitor, &info.x, &info.y);
    glfwGetMonitorPhysicalSize(monitor, &info.physicalWidthMM, &info.physicalHeightMM);
    glfwGetMonitorContentScale(monitor, &info.contentScaleX, &info.contentScaleY);

    if (const GLFWvidmode* mode = glfwGetVideoMode(monitor)) {
        info.currentVideoMode = toVideoMode(*mode);
    }

    return info;
}

uint32_t getWindowMonitorId(GLFWwindow* window)
{
    auto monitors = getOrderedMonitors();
    if (monitors.empty()) {
        return 0;
    }

    if (GLFWmonitor* fullscreenMonitor = glfwGetWindowMonitor(window)) {
        for (size_t i = 0; i < monitors.size(); ++i) {
            if (monitors[i] == fullscreenMonitor) {
                return static_cast<uint32_t>(i);
            }
        }
    }

    int windowX = 0;
    int windowY = 0;
    int windowWidth = 0;
    int windowHeight = 0;
    glfwGetWindowPos(window, &windowX, &windowY);
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    int bestArea = -1;
    uint32_t bestId = 0;

    for (size_t i = 0; i < monitors.size(); ++i) {
        int monitorX = 0;
        int monitorY = 0;
        glfwGetMonitorPos(monitors[i], &monitorX, &monitorY);

        const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
        if (!mode) {
            continue;
        }

        const int overlapLeft = std::max(windowX, monitorX);
        const int overlapTop = std::max(windowY, monitorY);
        const int overlapRight = std::min(windowX + windowWidth, monitorX + mode->width);
        const int overlapBottom = std::min(windowY + windowHeight, monitorY + mode->height);
        const int overlapWidth = std::max(0, overlapRight - overlapLeft);
        const int overlapHeight = std::max(0, overlapBottom - overlapTop);
        const int overlapArea = overlapWidth * overlapHeight;

        if (overlapArea > bestArea) {
            bestArea = overlapArea;
            bestId = static_cast<uint32_t>(i);
        }
    }

    return bestId;
}

}  // namespace cwin::backend::glfw
