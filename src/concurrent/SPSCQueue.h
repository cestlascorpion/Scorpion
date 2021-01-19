/*
 * Copyright (c) 2018 Erik Rigtorp <erik@rigtorp.se>
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * A single producer single consumer lock-free fixed size queue.
 */

#pragma once

#include <atomic>
#include <cassert>
#include <stdexcept>

namespace scorpion {

template <typename T>
class SPSCQueue {
public:
    explicit SPSCQueue(size_t capacity = kDefaultCapacity)
        : capacity_(capacity < kDefaultCapacity ? kDefaultCapacity : capacity)
        , slots_(capacity_ < 2 ? nullptr : static_cast<T *>(operator new[](sizeof(T) * (capacity_ + 2 * kPadding))))
        , head_(0)
        , tail_(0)
        , padding_() {
        assert(alignof(SPSCQueue<T>) >= kCacheLineSize);
        assert(reinterpret_cast<char *>(&tail_) - reinterpret_cast<char *>(&head_) >=
               static_cast<std::ptrdiff_t>(kCacheLineSize));
    }

    ~SPSCQueue() noexcept {
        while (Front()) {
            Pop();
        }
        operator delete[](slots_);
    }

    SPSCQueue(const SPSCQueue &) = delete;
    SPSCQueue operator=(const SPSCQueue &) = delete;

public:
    void Push(const T &v) noexcept(std::is_nothrow_copy_constructible<T>::value) {
        static_assert(std::is_copy_constructible<T>::value, "T must be copy constructible");
        Emplace(v);
    }

    template <typename P, typename = typename std::enable_if<std::is_constructible<T, P &&>::value>::type>
    void Push(P &&v) noexcept(std::is_nothrow_constructible<T, P &&>::value) {
        Emplace(std::forward<P>(v));
    }

    bool TryPush(const T &v) noexcept(std::is_nothrow_copy_constructible<T>::value) {
        static_assert(std::is_copy_constructible<T>::value, "T must be copy constructible");
        return TryEmplace(v);
    }

    template <typename P, typename = typename std::enable_if<std::is_constructible<T, P &&>::value>::type>
    bool TryPush(P &&v) noexcept(std::is_nothrow_constructible<T, P &&>::value) {
        return TryEmplace(std::forward<P>(v));
    }

    template <typename... Args>
    void Emplace(Args &&... args) noexcept(std::is_nothrow_constructible<T, Args &&...>::value) {
        static_assert(std::is_constructible<T, Args &&...>::value, "T must be constructible with Args&&...");
        auto const head = head_.load(std::memory_order_acquire);
        auto nextHead = head + 1;
        if (nextHead == capacity_) {
            nextHead = 0;
        }
        while (nextHead == tail_.load(std::memory_order_acquire)) {
        }
        new (&slots_[head + kPadding]) T(std::forward<Args>(args)...);
        head_.store(nextHead, std::memory_order_release);
    }

    template <typename... Args>
    bool TryEmplace(Args &&... args) noexcept(std::is_nothrow_constructible<T, Args &&...>::value) {
        static_assert(std::is_constructible<T, Args &&...>::value, "T must be constructible with Args&&...");
        auto const head = head_.load(std::memory_order_acquire);
        auto nextHead = head + 1;
        if (nextHead == capacity_) {
            nextHead = 0;
        }
        if (nextHead == tail_.load(std::memory_order_acquire)) {
            return false;
        }
        new (&slots_[head + kPadding]) T(std::forward<Args>(args)...);
        head_.store(nextHead, std::memory_order_release);
        return true;
    }

    void Pop(T &v) noexcept {
        static_assert(std::is_nothrow_destructible<T>::value, "T must be nothrow destructible");
        auto const tail = tail_.load(std::memory_order_acquire);
        while (head_.load(std::memory_order_acquire) == tail) {
        }
        v = slots_[tail + kPadding];
        slots_[tail + kPadding].~T();
        auto nextTail = tail + 1;
        if (nextTail == capacity_) {
            nextTail = 0;
        }
        tail_.store(nextTail, std::memory_order_release);
    }

    bool TryPop(T &v) noexcept {
        static_assert(std::is_nothrow_destructible<T>::value, "T must be nothrow destructible");
        auto const tail = tail_.load(std::memory_order_acquire);
        if (head_.load(std::memory_order_acquire) == tail) {
            return false;
        }
        v = slots_[tail + kPadding];
        slots_[tail + kPadding].~T();
        auto nextTail = tail + 1;
        if (nextTail == capacity_) {
            nextTail = 0;
        }
        tail_.store(nextTail, std::memory_order_release);
        return true;
    }

    T *Front() noexcept {
        auto const tail = tail_.load(std::memory_order_acquire);
        if (head_.load(std::memory_order_acquire) == tail) {
            return nullptr;
        }
        return &slots_[tail + kPadding];
    }

    void Pop() noexcept {
        static_assert(std::is_nothrow_destructible<T>::value, "T must be nothrow destructible");
        auto const tail = tail_.load(std::memory_order_acquire);
        assert(head_.load(std::memory_order_acquire) != tail);
        slots_[tail + kPadding].~T();
        auto nextTail = tail + 1;
        if (nextTail == capacity_) {
            nextTail = 0;
        }
        tail_.store(nextTail, std::memory_order_release);
    }

private:
    static constexpr size_t kDefaultCapacity = 256;

    static constexpr size_t kCacheLineSize = 128;

    // Padding to avoid false sharing between slots_ and adjacent allocations
    static constexpr size_t kPadding = (kCacheLineSize - 1) / sizeof(T) + 1;

private:
    const size_t capacity_;
    T *const slots_;

    // Align to avoid false sharing between head_ and tail_
    alignas(kCacheLineSize) std::atomic<size_t> head_;
    alignas(kCacheLineSize) std::atomic<size_t> tail_;

    // Padding to avoid adjacent allocations to share cache line with tail_
    char padding_[kCacheLineSize - sizeof(tail_)];
};

} // namespace scorpion
