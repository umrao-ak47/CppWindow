/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#ifndef CPPWINDOW_HEADER_WINDOW_INTERNAL_HPP
#define CPPWINDOW_HEADER_WINDOW_INTERNAL_HPP

#include "backend/window_desc.hpp"

namespace cwin {

class Window;

struct WindowAccess
{
    [[nodiscard]] static Window makeWindow(backend::WindowDesc desc);
};

}  // namespace cwin

#endif
