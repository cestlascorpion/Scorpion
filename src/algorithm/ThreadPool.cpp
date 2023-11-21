#include "ThreadPool.h"
#include "BlockingQueue.h"

#include <thread>

namespace scorpion {

using namespace std;
using cbType = function<int()>;

struct ThreadPool::Impl {
    static constexpr size_t kDefaultCapacity = 1024;

    atomic<bool> _running;
    vector<thread> _workers;
    unique_ptr<BlockingQueue<cbType>> _queue;

    explicit Impl(unsigned size)
        : _running(true)
        , _queue(new BlockingQueue<cbType>(kDefaultCapacity)) {
        for (auto i = 0u; i < size; i++) {
            _workers.emplace_back([this]() {
                while (_running.load(std::memory_order_relaxed)) {
                    cbType callback = nullptr;
                    if (!_queue->TryPop(callback)) {
                        continue;
                    }
                    try {
                        callback();
                    } catch (std::exception &e) {
                        printf("[Warn] task throw exception %s\n", e.what());
                    } catch (...) {
                        printf("[Warn] task throw non-std::exception\n");
                    }
                }
            });
        }
    }
    ~Impl() {
        _running.store(false, std::memory_order_relaxed);
        for (auto &t : _workers) {
            t.join();
        }
        cbType callback;
        while (_queue->TryPop(callback)) {
            try {
                callback();
            } catch (std::exception &e) {
                printf("[Warn] task throw exception %s\n", e.what());
            } catch (...) {
                printf("[Warn] task throw non-std::exception\n");
            }
        }
    }
};

ThreadPool::ThreadPool(unsigned int size)
    : _impl(new Impl(size)) {}

ThreadPool::~ThreadPool() = default;

bool ThreadPool::Push(function<int()> &&cb) {
    if (!_impl->_running.load(std::memory_order_relaxed)) {
        return false;
    }
    _impl->_queue->Push(std::forward<cbType>(cb));
    return true;
}

} // namespace scorpion