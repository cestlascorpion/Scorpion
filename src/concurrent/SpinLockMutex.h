/**
 * A simple implementation of spinlock with atomic flag.
 *
 */

#pragma once

#include <atomic>

namespace scorpion {

class SpinLockMutex {
public:
    SpinLockMutex()
        : flag(ATOMIC_FLAG_INIT) {}
    ~SpinLockMutex() = default;

public:
    void lock() {
        while (flag.test_and_set(std::memory_order::memory_order_acquire)) {
        }
    }
    void unlock() {
        flag.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag;
};

} // namespace scorpion
