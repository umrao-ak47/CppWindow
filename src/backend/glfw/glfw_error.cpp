/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#include "glfw_internal.hpp"

#include <string>
#include <utility>

namespace cwin::glfw_backend {

thread_local GlfwError g_LastGlfwError;

void glfwErrorCallback(int code, const char* description)
{
    g_LastGlfwError.code = code;
    g_LastGlfwError.description = description ? description : "";
}

GlfwError takeGlfwError()
{
    const char* description = nullptr;
    const int code = glfwGetError(&description);
    if (code != GLFW_NO_ERROR) {
        g_LastGlfwError = {};
        return GlfwError{
            .code = code,
            .description = description ? description : "",
        };
    }

    GlfwError stored = std::move(g_LastGlfwError);
    g_LastGlfwError = {};
    return stored;
}

void clearGlfwError()
{
    (void)takeGlfwError();
}

Error makeGlfwError(ErrorCode code, const std::string& message)
{
    GlfwError glfwError = takeGlfwError();
    std::string fullMessage = message;

    if (glfwError.code != GLFW_NO_ERROR) {
        fullMessage += " [GLFW code " + std::to_string(glfwError.code);
        if (!glfwError.description.empty()) {
            fullMessage += ": " + glfwError.description;
        }
        fullMessage += "]";
    }

    return Error(code, std::move(fullMessage));
}

Error makeGlfwError(ErrorCode code, const std::string& message, const std::string& detail)
{
    Error error = makeGlfwError(code, message);
    std::string fullMessage = error.what();
    if (!detail.empty()) {
        fullMessage += " [" + detail + "]";
    }
    return Error(code, std::move(fullMessage));
}

}  // namespace cwin::glfw_backend
