/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#include <memory>

#include "../../window_registry.hpp"
#include "glfw_internal.hpp"

namespace cwin::glfw_backend {

// store all refs to windows storage so we can clean it
// per frame
WindowStorageRegistry<WindowStorage> g_WindowRegistry;

void registerWindowStorage(const std::shared_ptr<WindowStorage>& storage)
{
    g_WindowRegistry.registerStorage(storage);
}

void resetWindowStorage() noexcept
{
    g_WindowRegistry.resetAll();
}

void dispatchEventToAllWindows(const Event& event)
{
    g_WindowRegistry.forEach([&](WindowStorage& storage) {
        storage.inputState.handleEvent(event);
        storage.eventQueue.push_back(event);
    });
}

}  // namespace cwin::glfw_backend
