/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#ifndef CPPWINDOW_HEADER_STATIC_LOOKUP_HPP
#define CPPWINDOW_HEADER_STATIC_LOOKUP_HPP

#include <array>
#include <concepts>
#include <cstddef>

namespace cwin {

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
        for (size_t i = 0; i < N; i++) {
            addEntry(mapping[i]);
        }
    }

    constexpr void addEntry(const Entry& entry)
    {
        if (!isWrapperInRange(entry.wrapperVal) || !isBackendInRange(entry.backendVal)) {
            return;
        }

        size_t wIdx = static_cast<size_t>(entry.wrapperVal) - WrapperMinVal;
        toBackendMap[wIdx] = entry.backendVal;

        size_t bIdx = static_cast<size_t>(entry.backendVal) - BackendMinVal;
        toWrapperMap[bIdx] = entry.wrapperVal;
    }

    // Two-way Keyboard
    [[nodiscard]] constexpr BackendType toBackend(WrapperType k) const noexcept
    {
        if (!isWrapperInRange(k)) {
            return Traits::BackendNone;
        }

        size_t idx = static_cast<size_t>(k) - WrapperMinVal;
        return toBackendMap[idx];
    }

    [[nodiscard]] constexpr WrapperType toWrapper(BackendType k) const noexcept
    {
        if (!isBackendInRange(k)) {
            return Traits::WrapperNone;
        }

        size_t idx = static_cast<size_t>(k) - BackendMinVal;
        return toWrapperMap[idx];
    }

private:
    [[nodiscard]] static constexpr bool isWrapperInRange(WrapperType value) noexcept
    {
        const size_t raw = static_cast<size_t>(value);
        return raw >= WrapperMinVal && raw <= WrapperMaxVal;
    }

    [[nodiscard]] static constexpr bool isBackendInRange(BackendType value) noexcept
    {
        const size_t raw = static_cast<size_t>(value);
        return raw >= BackendMinVal && raw <= BackendMaxVal;
    }
};

}  // namespace cwin

#endif
