#pragma once

#include <functional>
#include <memory>

namespace scorpion {

class ThreadPool {
public:
    explicit ThreadPool(unsigned size);
    ~ThreadPool();

public:
    bool Push(std::function<int()> cb);

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

} // namespace scorpion