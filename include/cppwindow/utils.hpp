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

template <typename T>
concept Indexable = requires(T v) {
    { static_cast<size_t>(v) } -> std::same_as<size_t>;
};

template <typename T>
concept StaticLookupTraits = requires {
    // required types
    typename T::WrapperType;
    typename T::BackendType;

    // types must be indexable
    requires Indexable<typename T::WrapperType>;
    requires Indexable<typename T::BackendType>;

    // required static bounds
    { T::WrapperNone } -> std::same_as<const typename T::WrapperType&>;
    { T::WrapperFirst } -> std::same_as<const typename T::WrapperType&>;
    { T::WrapperLast } -> std::same_as<const typename T::WrapperType&>;
    { T::BackendNone } -> std::same_as<const typename T::BackendType&>;
    { T::BackendFirst } -> std::same_as<const typename T::BackendType&>;
    { T::BackendLast } -> std::same_as<const typename T::BackendType&>;
};

template <StaticLookupTraits Traits>
struct StaticLookup
{
    using WrapperType = typename Traits::WrapperType;
    using BackendType = typename Traits::BackendType;

    static constexpr size_t WrapperMinVal = static_cast<size_t>(Traits::WrapperFirst);
    static constexpr size_t WrapperMaxVal = static_cast<size_t>(Traits::WrapperLast);
    static constexpr size_t WrapperCount = (WrapperMaxVal - WrapperMinVal + 1);

    static constexpr size_t BackendMinVal = static_cast<size_t>(Traits::BackendFirst);
    static constexpr size_t BackendMaxVal = static_cast<size_t>(Traits::BackendLast);
    static constexpr size_t BackendCount = (BackendMaxVal - BackendMinVal + 1);

    // Mapper -> Backend
    std::array<BackendType, WrapperCount> toBackendMap;
    // Backend -> Mapper
    std::array<WrapperType, BackendCount> toWrapperMap;

    struct Entry
    {
        WrapperType wrapperVal;
        BackendType backendVal;
    };

    template <size_t N>
    constexpr StaticLookup(const Entry (&mapping)[N])
        : toBackendMap{},
          toWrapperMap{}
    {
        toWrapperMap.fill(Traits::WrapperNone);
        toBackendMap.fill(Traits::BackendNone);
        for (int i = 0; i < N; i++) {
            addEntry(mapping[i]);
        }
        //(..., addEntry<mapping>());
    }

    constexpr void addEntry(const Entry& entry)
    {
        size_t wIdx = static_cast<size_t>(entry.wrapperVal) - WrapperMinVal;
        // static_assert(wIdx < WrapperMaxVal, "Wrapper out of range");
        toBackendMap[wIdx] = entry.backendVal;

        size_t bIdx = static_cast<size_t>(entry.backendVal) - BackendMinVal;
        // static_assert(bIdx < BackendMaxVal, "Backend out of range");
        toWrapperMap[bIdx] = entry.wrapperVal;
    }

    // Two-way Keyboard
    inline BackendType toBackend(WrapperType k) const noexcept
    {
        size_t idx = static_cast<size_t>(k) - WrapperMinVal;
        if (idx < 0 || idx > WrapperMaxVal) {
            return Traits::BackendNone;
        }
        return toBackendMap[idx];
    }

    inline WrapperType toWrapper(BackendType k) const noexcept
    {
        size_t idx = static_cast<size_t>(k) - BackendMinVal;
        if (idx < 0 || idx > BackendMaxVal) {
            return Traits::WrapperNone;
        }
        return toWrapperMap[k];
    }
};

}  // namespace cwin

#endif