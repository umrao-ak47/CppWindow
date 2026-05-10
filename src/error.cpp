/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#include <cppwindow/cppwindow.hpp>

#include <utility>

namespace cwin {

//----------------------------------------------------------------------------
//  Error Implementation
//----------------------------------------------------------------------------
Error::Error(ErrorCode code, std::string message)
    : std::runtime_error(std::move(message)),
      code_(code)
{
}

ErrorCode Error::code() const noexcept
{
    return code_;
}

}  // namespace cwin
