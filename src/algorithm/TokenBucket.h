#pragma once

#include <algorithm>
#include <chrono>

namespace scorpion {

class TokenBucket {
public:
    TokenBucket(int64_t size, int64_t rate)
        : _size(size)
        , _rate(rate)
        , _ts(std::chrono::steady_clock::now())
        , _token(size / 2) {}
    ~TokenBucket() = default;

public:
    bool grant() {
        auto now = std::chrono::steady_clock::now();
        auto in = (now - _ts).count() * _rate;
        _token = std::min(_size, _token + in);
        _ts = now;
        if (_token > 0) {
            --_token;
            return true; // passed
        } else {
            return false; // denied
        }
    }

private:
    int64_t _size;
    int64_t _rate;
    std::chrono::time_point<std::chrono::steady_clock> _ts;
    int64_t _token;
};

} // namespace scorpion
