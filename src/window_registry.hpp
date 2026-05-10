/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#ifndef CPPWINDOW_HEADER_WINDOW_REGISTRY_HPP
#define CPPWINDOW_HEADER_WINDOW_REGISTRY_HPP

#include <concepts>
#include <cstddef>
#include <memory>
#include <mutex>
#include <vector>

namespace cwin {

template <typename T>
concept Resettable = requires(T& value) {
    { value.reset() } noexcept -> std::same_as<void>;
};

template <Resettable T>
class WindowStorageRegistry
{
public:
    WindowStorageRegistry() = default;
    ~WindowStorageRegistry() = default;

    WindowStorageRegistry(const WindowStorageRegistry&) = delete;
    WindowStorageRegistry& operator=(const WindowStorageRegistry&) = delete;
    WindowStorageRegistry(WindowStorageRegistry&&) = delete;
    WindowStorageRegistry& operator=(WindowStorageRegistry&&) = delete;

    // Stores weak references to window-owned storage and compacts expired
    // entries during resetAll().
    void registerStorage(const std::shared_ptr<T>& storage)
    {
        std::scoped_lock lock(mtx_);
        if (tail_ < storageRefs_.size()) {
            storageRefs_[tail_] = storage;
        } else {
            storageRefs_.push_back(storage);
        }
        ++tail_;
    }

    // Calls T::reset() for each live entry and removes expired entries. reset()
    // is called while the registry mutex is held; re-entering this registry from
    // reset() can deadlock.
    void resetAll() noexcept
    {
        std::scoped_lock lock(mtx_);
        const std::size_t oldTail = tail_;
        std::size_t newTail = 0;

        for (std::size_t i = 0; i < oldTail; ++i) {
            if (auto storage = storageRefs_[i].lock()) {
                storage->reset();
                if (i != newTail) {
                    storageRefs_[newTail] = std::move(storageRefs_[i]);
                }
                ++newTail;
            }
        }

        for (std::size_t i = newTail; i < oldTail; ++i) {
            storageRefs_[i].reset();
        }

        tail_ = newTail;
    }

private:
    std::vector<std::weak_ptr<T>> storageRefs_;
    std::size_t tail_ = 0;
    std::mutex mtx_;
};

}  // namespace cwin

#endif
