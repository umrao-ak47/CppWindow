/*
 * Copyright (c) 2026 Amit Kumar
 * Licensed under the MIT License.
 */

#pragma once

#include <mutex>
#include <vector>

namespace cppwindow {

template <typename T>
concept TypeHasResetFn = requires(T t) {
    { t.reset() };
};

template <TypeHasResetFn T>
class WindowStorageRegistry
{
public:
    void registerWindow(const std::shared_ptr<T>& storage)
    {
        std::lock_guard lock(mtx_);
        if (tail_ < storageRefs_.size()) {
            storageRefs_[tail_] = storage;
        } else {
            storageRefs_.push_back(storage);
        }
        ++tail_;
    }

    void resetAll()
    {
        std::lock_guard lock(mtx_);
        size_t newTail = 0;

        for (size_t i = 0; i < tail_; ++i) {
            if (auto s = storageRefs_[i].lock()) {
                s->reset();
                // keep live entries at front
                if (i != newTail) {
                    storageRefs_[newTail] = storageRefs_[i];
                }
                ++newTail;
            }
            // expired entries are ignored
        }
        // set tail to first free slot
        tail_ = newTail;
    }

private:
    std::vector<std::weak_ptr<T>> storageRefs_;
    size_t tail_ = 0;
    std::mutex mtx_;
};

}  // namespace cppwindow
