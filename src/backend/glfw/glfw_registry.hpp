/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 * * Note: The implementation utilizes GLFW (zlib license).
 */

#ifndef CPPWINDOW_HEADER_GLFW_REGISTRY_HPP
#define CPPWINDOW_HEADER_GLFW_REGISTRY_HPP

#include <memory>

namespace cwin {
class Event;
}

namespace cwin::backend::glfw {

class WindowStorage;

void registerWindowStorage(const std::shared_ptr<WindowStorage>& storage);
void resetWindowStorage() noexcept;
void dispatchEventToAllWindows(const Event& event);

}  // namespace cwin::backend::glfw

#endif
