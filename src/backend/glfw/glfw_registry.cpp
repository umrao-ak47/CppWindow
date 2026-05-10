/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#include "glfw_registry.hpp"

#include <memory>

#include "../../window_registry.hpp"
#include "glfw_window.hpp"

namespace cwin::backend::glfw {

namespace {

// store all refs to windows storage so we can clean it
// per frame
WindowStorageRegistry<WindowStorage> g_WindowRegistry;

}  // namespace

void registerWindowStorage(const std::shared_ptr<WindowStorage>& storage)
{
    g_WindowRegistry.registerStorage(storage);
}

void resetWindowStorage() noexcept
{
    g_WindowRegistry.resetAll();
}

}  // namespace cwin::backend::glfw
