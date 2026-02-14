/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#ifndef CPPWINDOW_HEADER_UTILS_HPP
#define CPPWINDOW_HEADER_UTILS_HPP

#include <array>
#include <memory>

namespace cwin {

template <class... Ts>
struct Visitor : Ts...
{
    using Ts::operator()...;
};

// template <class... Ts>
// Visitor(Ts...) -> Visitor<Ts...>;

// General traits - will be specialized
template <typename T, typename U>
struct InputTraits;

template <typename T, typename U>
struct StaticLookup
{
    using Traits = InputTraits<T, U>;

    // Mapper -> GLFW
    std::array<U, Traits::WrapperMax> toBackendMap{};
    // GLFW -> Mapper
    std::array<T, Traits::BackendMax> toWrapperMap{};

    struct Entry
    {
        T wrapperVal;
        U backendVal;
    };

    constexpr StaticLookup(std::initializer_list<Entry> mappings)
    {
        toWrapperMap.fill(Traits::WrapperNone);
        toBackendMap.fill(Traits::BackendNone);

        for (const auto& m : mappings) {
            // Fill Forward Map
            toBackendMap[static_cast<size_t>(m.wrapperVal)] = m.backendVal;

            // Fill Reverse Map (only if within bounds)
            if (m.backendVal >= Traits::BackendMin && m.backendVal < Traits::BackendMax) {
                toWrapperMap[static_cast<size_t>(m.backendVal)] = m.wrapperVal;
            }
        }
    }

    // Two-way Keyboard
    inline int toBackend(T k) const noexcept
    {
        return toBackendMap[static_cast<int>(k)];
    }

    inline T toWrapper(int k) const noexcept
    {
        return toWrapperMap[k];
    }
};

}  // namespace cwin

#endif