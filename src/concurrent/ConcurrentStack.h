/**
 * A implementation of concurrent stack.
 *
 */

#pragma once

#include <memory>
#include <mutex>
#include <stack>

namespace scorpion {

template <typename T>
class ConcurrentStack {
public:
    ConcurrentStack() = default;
    ~ConcurrentStack() = default;

    ConcurrentStack(const ConcurrentStack &other) {
        std::lock_guard<std::mutex> lock(other._mtx);
        _data = other._data;
    };
    ConcurrentStack &operator=(const ConcurrentStack &) = delete;

public:
    void Push(T val) {
        std::lock_guard<std::mutex> lock(_mtx);
        _data.push(std::move(val));
    }

    std::shared_ptr<T> Pop() {
        std::lock_guard<std::mutex> lock(_mtx);
        if (_data.empty()) {
            return std::shared_ptr<T>();
        }

        const std::shared_ptr<T> res(std::make_shared<T>(std::move(_data.top())));
        _data.pop();
        return res;
    }

    bool Pop(T &val) {
        std::lock_guard<std::mutex> lock(_mtx);
        if (_data.empty()) {
            return false;
        }

        val = std::move(_data.top());
        _data.pop();
        return true;
    }

    bool Empty() const {
        std::lock_guard<std::mutex> lock(_mtx);
        return _data.empty();
    }

    size_t Size() const {
        std::lock_guard<std::mutex> lock(_mtx);
        return _data.size();
    }

private:
    std::stack<T> _data;
    mutable std::mutex _mtx;
};

} // namespace scorpion
