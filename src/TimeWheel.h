/**
 * A raw implementation of timewheel and its wrapper.
 * Note that: better not use it when it comes to dealing with lots os task
 *
 */

#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

namespace scorpion {

class TimeWheelRaw {
public:
    explicit TimeWheelRaw(bool async = false);
    ~TimeWheelRaw();

    TimeWheelRaw(const TimeWheelRaw &) = delete;
    TimeWheelRaw &operator=(const TimeWheelRaw &) = delete;

public:
    void Add(std::function<void() noexcept> &&cb, unsigned int interval, int loop) noexcept;
    void Tick() noexcept;
    void Dump() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

template <typename Resolution = std::chrono::seconds>
class TimeWheel {
public:
    explicit TimeWheel(bool async = false)
        : _twr(new TimeWheelRaw(async))
        , _running(true)
        , _thread([this]() {
            while (_running.load(std::memory_order_acquire)) {
                std::this_thread::sleep_for(Resolution(1));
                std::lock_guard<std::mutex> lock(_mutex);
                _twr->Tick();
            }
        }) {}
    ~TimeWheel() {
        _running.store(false, std::memory_order_release);
        if (_thread.joinable()) {
            _thread.join();
        }
    }

    TimeWheel(const TimeWheel &) = delete;
    TimeWheel &operator=(const TimeWheel &) = delete;

public:
    void Add(std::function<void() noexcept> &&cb, unsigned int interval, int loop) noexcept {
        std::lock_guard<std::mutex> lock(_mutex);
        _twr->Add(std::forward<std::function<void() noexcept>>(cb), interval, loop);
    }

private:
    std::unique_ptr<TimeWheelRaw> _twr;
    std::atomic<bool> _running;
    std::mutex _mutex;
    std::thread _thread;
};

} // namespace scorpion