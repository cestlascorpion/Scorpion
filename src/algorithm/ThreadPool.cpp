#include "ThreadPool.h"
#include "AsyncTaskPool.h"

namespace scorpion {

using namespace std;
using cbType = function<int()>;

struct ThreadPool::Impl {
    unique_ptr<Manager<Task, MPMCQueue<Task>>> _manager;

    Impl(unsigned size, unsigned capacity)
        : _manager(new Manager<Task, MPMCQueue<Task>>()) {
        _manager->Init(size, capacity, 20, 1000 * 10);
    }
    ~Impl() {
        _manager->Final(true);
    }
};

ThreadPool::ThreadPool(unsigned size, unsigned capacity)
    : _impl(new Impl(size, capacity / size)) {}

ThreadPool::~ThreadPool() = default;

bool ThreadPool::Add(cbType &&cb) {
    auto id = unsigned(time(nullptr));
    return _impl->_manager->Submit(id, Task(id, chrono::steady_clock::now(), forward<cbType>(cb)));
}

} // namespace scorpion