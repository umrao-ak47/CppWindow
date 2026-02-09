/*
 * Copyright (c) 2026 Your Name
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#include <glfw-vk/Context.hpp>

#include <optional>

#include "internal/GlfwIncludes.hpp"

namespace glfwvk {

GlfwContext& GlfwContext::Get()
{
    // init glfw context
    static GlfwContext instance;
    return instance;
}

GlfwContext::GlfwContext()
{
    if (!glfwInit()) {
        throw GLFWException("Failed to initialize GLFW");
    }
}

GlfwContext::~GlfwContext()
{
    glfwTerminate();
}

bool GlfwContext::isVulkanSupported() const
{
    return glfwVulkanSupported();
}

std::vector<std::string> GlfwContext::getRequiredGlfwVulkanExtensions() const
{
    uint32_t count = 0;
    const char** ext = glfwGetRequiredInstanceExtensions(&count);

    return std::vector<std::string>(ext, ext + count);
}

}  // namespace glfwvk
