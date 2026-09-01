/**
 * A implementation of token-bucket algorithm.
 *
 */

#pragma once

#include <algorithm>
#include <chrono>

namespace scorpion {

class TokenBucket {
public:
    TokenBucket(int64_t size, double rate)
        : _size(size)
        , _rate(rate)
        , _ts(std::chrono::steady_clock::now())
        , _token((double)size / 2.0) {}
    ~TokenBucket() = default;

public:
    bool grant() {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration<double>(now - _ts).count();
        _token = std::min((double)_size, _token + elapsed * _rate);
        _ts = now;
        if (_token >= 1.0) {
            --_token;
            return true; // passed
        } else {
            return false; // denied
        }
    }

private:
    int64_t _size;
    double _rate;
    std::chrono::time_point<std::chrono::steady_clock> _ts;
    double _token;
};

} // namespace scorpion
