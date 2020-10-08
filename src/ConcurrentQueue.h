#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

namespace scorpion {

template <typename T>
class ConcurrentQueue {
public:
    ConcurrentQueue() = default;
    ~ConcurrentQueue() = default;

    ConcurrentQueue(const ConcurrentQueue &other) {
        std::lock_guard<std::mutex> lock(other._mtx);
        _data = other._data;
    };
    ConcurrentQueue &operator=(const ConcurrentQueue &) = delete;

public:
    void Push(T val) {
        std::lock_guard<std::mutex> lock(_mtx);
        _data.push(std::move(val));
        _cond.notify_one();
    }

    std::shared_ptr<T> Pop() {
        std::unique_lock<std::mutex> lock(_mtx);
        _cond.wait(lock, [this]() -> bool { return !_data.empty(); });
        std::shared_ptr<T> res(std::make_shared<T>(std::move(_data.front())));
        _data.pop();
        return res;
    }

    void Pop(T &val) {
        std::unique_lock<std::mutex> lock(_mtx);
        _cond.wait(lock, [this]() -> bool { return !_data.empty(); });
        val = std::move(_data.front());
        _data.pop();
    }

    std::shared_ptr<T> TryPop() {
        std::lock_guard<std::mutex> lock(_mtx);
        if (_data.empty()) {
            return std::shared_ptr<T>();
        }
        std::shared_ptr<T> res(std::make_shared<T>(std::move(_data.front())));
        _data.pop();
        return res;
    }

    bool TryPop(T &val) {
        std::lock_guard<std::mutex> lock(_mtx);
        if (_data.empty()) {
            return false;
        }

        val = std::move(_data.front());
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
    std::queue<T> _data;
    mutable std::mutex _mtx;
    std::condition_variable _cond;
};

} // namespace scorpion
