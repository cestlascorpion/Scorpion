#include "TimeWheel.h"
#include "ThreadPool.h"

#include <list>
#include <memory>
#include <vector>

namespace scorpion {

using namespace std;
using cbType = function<int()>;

constexpr unsigned kTimeWheelSpan = 1;
constexpr unsigned kTimeWheelSize = 10;

struct CEvent {
    unsigned _interval;
    unsigned _rotation;
    int _loop;
    cbType _callback;

    CEvent(unsigned interval, unsigned rotation, int loop, cbType &&cb)
        : _interval(interval)
        , _rotation(rotation)
        , _loop(loop)
        , _callback(forward<cbType>(cb)) {}

    CEvent(const CEvent &) = delete;
    CEvent &operator=(const CEvent &) = delete;
};

struct TimeWheelRaw::Impl {
    const bool _async;
    const unsigned _span;
    const unsigned _size;
    unsigned _cursor;
    vector<list<unique_ptr<CEvent>>> _slots;
    unique_ptr<ThreadPool> _pool;

    explicit Impl(bool async, unsigned span, unsigned size)
        : _async(async)
        , _span(span)
        , _size(size)
        , _cursor(0)
        , _pool(nullptr) {
        _slots.resize(size);
        if (_async) {
            _pool = std::make_unique<ThreadPool>(3,1204);
        }
    }
};

} // namespace scorpion

namespace scorpion {

TimeWheelRaw::TimeWheelRaw(bool async)
    : _impl(new Impl(async, kTimeWheelSpan, kTimeWheelSize)) {}

TimeWheelRaw::~TimeWheelRaw() = default;

void TimeWheelRaw::Add(cbType &&cb, unsigned interval, int loop) {
    auto ticks = interval < _impl->_span ? 1u : interval / _impl->_span;
    auto rotation = ticks / _impl->_size;
    auto index = (_impl->_cursor + ticks % _impl->_size) % _impl->_size;
    auto event = make_unique<CEvent>(interval, rotation, loop, forward<cbType>(cb));
    _impl->_slots[index].push_back(move(event));
}

void TimeWheelRaw::Tick() {
    auto &list = _impl->_slots[_impl->_cursor];
    for (auto iter = list.begin(); iter != list.end(); /*nothing*/) {
        if ((*iter)->_rotation == 0) {
            if (_impl->_async) {
                // bug with async(): temporary's dtor waits for (*iter)->_callback()
                // async(launch::async, (*iter)->_callback);

                // Note: start new thread is a bad idea, thread pool may be a good solution.
                // thread t((*iter)->_callback);
                // t.detach();

                // how about a thread pool?
                auto cb = (*iter)->_callback; // copy
                _impl->_pool->Add(forward<cbType>(cb));
            } else {
                (*iter)->_callback();
            }
            if ((*iter)->_loop < 0) {
                auto interval = (*iter)->_interval;
                this->Add(move((*iter)->_callback), interval, -1);
            } else {
                auto loop = --(*iter)->_loop;
                if (loop > 0) {
                    auto interval = (*iter)->_interval;
                    this->Add(move((*iter)->_callback), interval, loop);
                }
            }
            iter = list.erase(iter);
        } else {
            --(*iter)->_rotation;
            ++iter;
        }
    }
    _impl->_cursor = (_impl->_cursor + 1u) % _impl->_size;
}

void TimeWheelRaw::Dump() const {
    for (const auto &list : _impl->_slots) {
        for (const auto &event : list) {
            printf("[event: rotation %u]\t", event->_rotation);
        }
        printf("\n");
    }
}

} // namespace scorpion
