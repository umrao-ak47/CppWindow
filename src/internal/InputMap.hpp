#pragma once

#include <glfw-vk/KeyDefs.hpp>

#include <array>

#include "GlfwIncludes.hpp"

namespace glfwvk {

// General traits - will be specialized
template <typename T>
struct InputTraits;

template <>
struct InputTraits<Key>
{
    static constexpr Key None = Key::Unknown;
    static constexpr int GlfwMax = GLFW_KEY_LAST;
    static constexpr unsigned int Count = KeyCount;
};

template <>
struct InputTraits<MouseButton>
{
    static constexpr MouseButton None = MouseButton::Left;
    static constexpr int GlfwMax = GLFW_MOUSE_BUTTON_LAST;
    static constexpr unsigned int Count = MouseButtonCount;
};

template <typename T>
struct StaticLookup
{
    using Traits = InputTraits<T>;

    // Mapper -> GLFW
    std::array<int, Traits::Count> toGlfwMap{};
    // GLFW -> Mapper
    std::array<T, Traits::GlfwMax + 1> toMapperMap{};

    struct Entry
    {
        T mapperVal;
        int glfwVal;
    };

    constexpr StaticLookup(std::initializer_list<Entry> mappings)
    {
        toMapperMap.fill(Traits::None);
        toGlfwMap.fill(0);

        for (const auto& m : mappings) {
            // Fill Forward Map
            toGlfwMap[static_cast<size_t>(m.mapperVal)] = m.glfwVal;

            // Fill Reverse Map (only if within bounds)
            if (m.glfwVal >= 0 && m.glfwVal <= Traits::GlfwMax) {
                toMapperMap[static_cast<size_t>(m.glfwVal)] = m.mapperVal;
            }
        }
    }

    // Two-way Keyboard
    inline int toGlfw(T k) const noexcept
    {
        return toGlfwMap[static_cast<int>(k)];
    }

    inline T toMapper(int k) const noexcept
    {
        return toMapperMap[k];
    }
};

class InputMap
{
public:
    static int toGlfwKey(Key k);
    static Key toKey(int k);
    static int toGlfwMouseButton(MouseButton b);
    static MouseButton toMouseButton(int b);
};

}  // namespace glfwvk
