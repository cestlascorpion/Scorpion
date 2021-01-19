#pragma once

#include <algorithm>
#include <chrono>

namespace scorpion {

class LeakyBucket {
public:
    LeakyBucket(int64_t size, int64_t rate)
        : _size(size)
        , _rate(rate)
        , _ts(std::chrono::steady_clock::now())
        , _water(size / 2) {}
    ~LeakyBucket() = default;

public:
    bool grant() {
        auto now = std::chrono::steady_clock::now();
        auto out = (now - _ts).count() * _rate;
        _water = std::max(0l, _water - out);
        _ts = now;
        if (_water + 1l < _size) {
            ++_water;
            return true; // passed
        } else {
            return false; // denied
        }
    }

private:
    int64_t _size;
    int64_t _rate;
    std::chrono::time_point<std::chrono::steady_clock> _ts;
    int64_t _water;
};

} // namespace scorpion
