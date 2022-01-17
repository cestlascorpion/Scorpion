/**
 * specialization version: Worker has a private mpmc queue and observes another one.
 *
 * Key Features:
 * 1. mpmc + stealing -> task is executed out of order.
 * 2. Submitted task will be assigned with the round-robin algorithm.
 * 3. Each worker has its own queue, it also can steal task from next worker if necessary.
 * 4. Worker can be reused as "Start()->Stop()->Start()->..." (better not do that).
 * 5. Call Stop(clean = true) to make sure no task is left behind (destructor will not do that).
 *
 */

/**
 * specialization version: Worker has a private mpmc queue.
 *
 * Key Features:
 * 1. mpsc -> task of the same queue is executed in order.
 * 2. Submitted task will be assigned with the hash algorithm.
 * 3. Each worker has its own queue which is private.
 * 4. Worker can be reused as "Start()->Stop()->Start()->..." (better not do that).
 * 5. Call Stop(clean = true) to make sure no task is left behind (destructor will not do that).
 *
 */

#pragma once

#include "AsyncTaskPoolTemplate.h"
#include "MPMCQueue.h"
#include "MPSCQueue.h"

#include <atomic>
#include <cassert>
#include <chrono>
#include <exception>
#include <functional>
#include <thread>

namespace scorpion {

class Task {
public:
    Task()
        : _id(0)
        , _ts(std::chrono::steady_clock::now()) {}

    explicit Task(unsigned id, std::chrono::time_point<std::chrono::steady_clock> ts, std::function<int()> &&func)
        : _id(id)
        , _ts(ts)
        , _func(std::forward<std::function<int()>>(func)) {}

    // non copyable
    Task(const Task &) = delete;
    Task &operator=(const Task &) = delete;

    // only movable
    Task(Task &&t) noexcept {
        _id = t._id;
        _ts = t._ts;
        _func = std::move(t._func);
    }
    Task &operator=(Task &&t) noexcept {
        _id = t._id;
        _ts = t._ts;
        _func = std::move(t._func);
        return *this;
    }

public:
    unsigned _id;
    std::chrono::time_point<std::chrono::steady_clock> _ts;
    std::function<int()> _func;
};

} // namespace scorpion

namespace scorpion {

template <>
class Worker<Task, MPMCQueue<Task>> {
public:
    using queue = MPMCQueue<Task>;

public:
    Worker(unsigned id, unsigned sleep, unsigned timeout, queue *local, queue *steal)
        : _id(id)
        , _sleep(sleep)
        , _timeout(timeout)
        , _local(local)
        , _steal(steal)
        , _running(false) {
        assert(local != nullptr);
    }

    virtual ~Worker() {
        _running = false;
        if (_thread.joinable()) {
            _thread.join();
        }
    }

public:
    virtual bool Start() {
        if (_running) {
            printf("[Warn] worker %u is running\n", _id);
            return false;
        }
        _running = true;
        _thread = std::thread([this]() {
            while (_running) {
                Task task;
                if (_local->TryPop(task)) {
                    execute(task);
                    continue;
                }
                if (_steal != nullptr && _steal->TryPop(task)) {
                    execute(task);
                    continue;
                }
                if (_sleep > 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(_sleep));
                } else {
                    std::this_thread::yield();
                }
            }
        });
        return true;
    }

    virtual bool Stop(bool clean) {
        if (!_running) {
            printf("[Warn] worker %u is not running\n", _id);
            return false;
        }
        _running = false;
        if (_thread.joinable()) {
            _thread.join();
        }
        if (clean) {
            Task task;
            while (_local->TryPop(task)) {
                execute(task);
            }
        }
        return true;
    }

    virtual bool Add(Task &&task) {
        if (_local == nullptr) {
            return false;
        }
        unsigned count = 0;
        while (!_local->TryPush(std::forward<Task>(task))) {
            if (++count > 3) {
                printf("[Warn] queue %u is full\n", _id);
                return false;
            }
            std::this_thread::yield();
        }
        return true;
    }

protected:
    virtual void execute(const Task &task) {
        auto now = std::chrono::steady_clock::now();
        auto wait = std::chrono::duration_cast<std::chrono::milliseconds>(now - task._ts).count();
        if (wait < _timeout) {
            try {
                task._func();
            } catch (std::exception &e) {
                printf("[Warn] task throw exception %s\n", e.what());
            } catch (...) {
                printf("[Warn] task throw non-std::exception\n");
            }
        } else {
            printf("[Warn] task timeout %u wait %lld ms\n", task._id, wait);
        }
    };

protected:
    const unsigned _id;
    const unsigned _sleep;
    const unsigned _timeout;

    queue *const _local;
    queue *const _steal;

    mutable bool _running;
    std::thread _thread;
};

template <>
class Manager<Task, MPMCQueue<Task>> {
public:
    using queue = MPMCQueue<Task>;

public:
    Manager() = default;
    virtual ~Manager() = default;

public:
    virtual bool Init(unsigned pool_size, unsigned queue_len, unsigned sleep, unsigned timeout) {
        _queues.reserve(pool_size);
        for (unsigned idx = 0; idx < pool_size; ++idx) {
            std::unique_ptr<queue> q(new MPMCQueue<Task>(queue_len));
            if (q == nullptr) {
                return false;
            }
            _queues.push_back(std::move(q));
        }
        _workers.reserve(pool_size);
        for (unsigned idx = 0; idx < pool_size; ++idx) {
            auto steal = (idx + pool_size - 1) % pool_size;
            std::unique_ptr<Worker<Task, queue>> worker(
                new Worker<Task, queue>(idx, sleep, timeout, _queues[idx].get(), _queues[steal].get()));
            if (worker == nullptr) {
                return false;
            }
            _workers.push_back(std::move(worker));
        }
        for (auto &worker : _workers) {
            if (!worker->Start()) {
                return false;
            }
        }
        return true;
    }

    virtual void Final(bool clean) {
        for (auto &worker : _workers) {
            if (worker != nullptr) {
                worker->Stop(clean);
            }
        }
    }

    virtual bool Submit(unsigned uid, Task &&task) {
        unsigned try_count = 0;
        unsigned route_id = route(uid);
        while (try_count < _queues.size()) {
            unsigned id = (route_id + try_count) % (unsigned)_queues.size();
            if (_workers[id]->Add(std::forward<Task>(task))) {
                return true;
            }
            ++try_count;
        }
        printf("[Warn] all the queues are full\n");
        return false;
    }

protected:
    inline unsigned route(unsigned) const {
        static std::atomic<unsigned> round(0);
        return round.fetch_add(1) % (unsigned)_queues.size();
    }

protected:
    std::vector<std::unique_ptr<queue>> _queues;
    std::vector<std::unique_ptr<Worker<Task, queue>>> _workers;
};

} // namespace scorpion

namespace scorpion {

template <>
class Worker<Task, MPSCQueue<Task>> {
public:
    using queue = MPSCQueue<Task>;

public:
    Worker(unsigned id, unsigned sleep, unsigned timeout, queue *local)
        : _id(id)
        , _sleep(sleep)
        , _timeout(timeout)
        , _local(local)
        , _running(false) {
        assert(local != nullptr);
    }

    virtual ~Worker() {
        _running = false;
        if (_thread.joinable()) {
            _thread.join();
        }
    }

public:
    virtual bool Start() {
        if (_running) {
            printf("[Warn] worker %u is running\n", _id);
            return false;
        }
        _running = true;
        _thread = std::thread([this]() {
            while (_running) {
                auto bulk = _local->TryPopBulk();
                if (!bulk.empty()) {
                    for (const auto &task : bulk) {
                        execute(task);
                    }
                    continue;
                }
                if (_sleep > 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(_sleep));
                } else {
                    std::this_thread::yield();
                }
            }
        });

        return true;
    }

    virtual bool Stop(bool clean) {
        if (!_running) {
            printf("[Warn] worker %u is not running\n", _id);
            return false;
        }
        _running = false;
        if (_thread.joinable()) {
            _thread.join();
        }
        if (clean) {
            Task task;
            while (_local->TryPop(task)) {
                execute(task);
            }
        }
        return true;
    }

    virtual bool Add(Task &&task) {
        if (_local == nullptr) {
            return false;
        }
        unsigned count = 0;
        while (!_local->TryPush(std::forward<Task>(task))) {
            if (++count > 3) {
                printf("[Warn] queue %u is full\n", _id);
                return false;
            }
            std::this_thread::yield();
        }
        return true;
    }

protected:
    virtual void execute(const Task &task) {
        auto now = std::chrono::steady_clock::now();
        auto wait = std::chrono::duration_cast<std::chrono::milliseconds>(now - task._ts).count();
        if (wait < _timeout) {
            try {
                task._func();
            } catch (std::exception &e) {
                printf("[Warn] task throw exception %s\n", e.what());
            } catch (...) {
                printf("[Warn] task throw non-std::exception\n");
            }
        } else {
            printf("[Warn] task timeout %u wait %lld ms\n", task._id, wait);
        }
    };

protected:
    unsigned _id;
    unsigned _sleep;
    unsigned _timeout;

    queue *_local;

    bool _running;
    std::thread _thread;
};

template <>
class Manager<Task, MPSCQueue<Task>> {
public:
    using queue = MPSCQueue<Task>;

public:
    Manager() = default;
    virtual ~Manager() = default;

public:
    virtual bool Init(unsigned pool_size, unsigned queue_len, unsigned sleep, unsigned timeout) {
        _queues.reserve(pool_size);
        for (unsigned idx = 0; idx < pool_size; ++idx) {
            std::unique_ptr<queue> q(new MPSCQueue<Task>(queue_len));
            if (q == nullptr) {
                return false;
            }
            _queues.push_back(std::move(q));
        }
        _workers.reserve(pool_size);
        for (unsigned idx = 0; idx < pool_size; ++idx) {
            std::unique_ptr<Worker<Task, queue>> worker(
                new Worker<Task, queue>(idx, sleep, timeout, _queues[idx].get()));
            if (worker == nullptr) {
                return false;
            }
            _workers.push_back(std::move(worker));
        }
        for (auto &worker : _workers) {
            if (!worker->Start()) {
                return false;
            }
        }
        return true;
    }

    virtual void Final(bool clean) {
        for (auto &worker : _workers) {
            if (worker != nullptr) {
                worker->Stop(clean);
            }
        }
    }

    virtual bool Submit(unsigned uid, Task &&task) {
        return _workers[route(uid)]->Add(std::forward<Task>(task));
    }

protected:
    inline unsigned route(unsigned uid) const {
        return uid % (unsigned)_queues.size();
    }

protected:
    std::vector<std::unique_ptr<queue>> _queues;
    std::vector<std::unique_ptr<Worker<Task, queue>>> _workers;
};

} // namespace scorpion
