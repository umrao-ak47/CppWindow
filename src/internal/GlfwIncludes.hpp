/*
 * Copyright (c) 2026 Your Name
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#pragma once

// Prevent GLFW from including OpenGL headers
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <format>
#include <stdexcept>
#include <string>

class GLFWException : public std::runtime_error
{
public:
    // Accept a custom message, default to empty
    explicit GLFWException(const std::string& customMsg = "")
        : std::runtime_error(formatError(customMsg))
    {
    }

private:
    static std::string formatError(const std::string& customMsg)
    {
        const char* description = nullptr;
        int errorCode = glfwGetError(&description);

        std::string finalMsg = "GLFW Exception";
        if (!customMsg.empty()) {
            finalMsg += " [" + customMsg + "]";
        }

        if (description) {
            finalMsg +=
                ": " + std::string(description) + " (Code: " + std::to_string(errorCode) + ")";
        } else {
            finalMsg += ": No specific GLFW error reported.";
        }

        return finalMsg;
    }
};