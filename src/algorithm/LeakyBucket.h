/**
 * A implementation of leaky-bucket algorithm.
 *
 */

#pragma once

#include <algorithm>
#include <chrono>

namespace scorpion {

class LeakyBucket {
public:
    LeakyBucket(int64_t size, double rate)
        : _size(size)
        , _rate(rate)
        , _ts(std::chrono::steady_clock::now())
        , _water((double)size / 2.0) {}
    ~LeakyBucket() = default;

public:
    bool grant() {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration<double>(now - _ts).count();
        _water = std::max(0.0, _water - elapsed * _rate);
        _ts = now;
        if (_water + 1.0 < static_cast<double>(_size)) {
            ++_water;
            return true; // passed
        } else {
            return false; // denied
        }
    }

private:
    int64_t _size;
    double _rate;
    std::chrono::time_point<std::chrono::steady_clock> _ts;
    double _water;
};

} // namespace scorpion
