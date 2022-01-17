#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

namespace scorpion {

template <typename T>
class BlockingQueue {
public:
    explicit BlockingQueue(size_t capacity)
        : _capacity(capacity)
        , _head(new Node)
        , _tail(_head)
        , _size(0) {}

    ~BlockingQueue() {
        T v;
        while (TryPop(v)) {
        }
        delete _head;
    }

    BlockingQueue(const BlockingQueue &other) = delete;
    BlockingQueue &operator=(const BlockingQueue &other) = delete;

public:
    void Push(const T &v) noexcept {
        static_assert(std::is_nothrow_copy_constructible<T>::value, "T must be nothrow copy constructible");
        Emplace(v);
    }

    template <typename P, typename = typename std::enable_if<std::is_nothrow_constructible<T, P &&>::value>::type>
    void Push(P &&v) noexcept {
        Emplace(std::forward<P>(v));
    }

    template <typename... Args>
    void Emplace(Args &&...args) noexcept {
        static_assert(std::is_nothrow_constructible<T, Args &&...>::value,
                      "T must be nothrow constructible with Args&&...");
        std::unique_lock<std::mutex> lock(_tail_mtx);
        _not_full.wait(lock, [this]() { return _size.load(std::memory_order_relaxed) < _capacity; });
        _tail->value = T(std::forward<Args>(args)...);
        Node *new_tail = new Node;
        _tail->next = new_tail;
        _tail = new_tail;
        lock.unlock();

        _size.fetch_add(1, std::memory_order_relaxed);
        _not_empty.notify_one();
    }

    void Pop(T &v) noexcept {
        std::unique_lock<std::mutex> lock(_head_mtx);
        _not_empty.wait(lock, [this]() { return _head != get_tail(); });
        Node *old_head = _head;
        v = std::move(old_head->value);
        _head = old_head->next;
        lock.unlock();

        _size.fetch_sub(1);
        _not_full.notify_one();
        delete old_head;
    }

    bool TryPop(T &v) noexcept {
        std::unique_lock<std::mutex> lock(_head_mtx);
        if (_head == get_tail()) {
            return false;
        }

        Node *old_head = _head;
        v = std::move(_head->value);
        _head = old_head->next;
        lock.unlock();

        _size.fetch_sub(1);
        _not_full.notify_one();
        delete old_head;
        return true;
    }

    size_t Size() const {
        return _size.load(std::memory_order_relaxed);
    }

    bool Empty() const {
        std::lock_guard<std::mutex> lock(_head_mtx);
        return _head == get_tail();
    }

private:
    static_assert(std::is_nothrow_copy_assignable<T>::value || std::is_nothrow_move_assignable<T>::value,
                  "T must be nothrow copy or move assignable");
    static_assert(std::is_nothrow_destructible<T>::value, "T must be nothrow destructible");

    struct Node {
        T value;
        Node *next;

        Node() = default;
        explicit Node(T v)
            : value(std::move(v))
            , next(nullptr) {}
    };

    Node *get_tail() const {
        std::lock_guard<std::mutex> lock(_tail_mtx);
        return _tail;
    }

private:
    const size_t _capacity;
    Node *_head;
    Node *_tail;
    std::atomic<size_t> _size;
    mutable std::mutex _head_mtx;
    mutable std::mutex _tail_mtx;
    std::condition_variable _not_empty;
    std::condition_variable _not_full;
};

} // namespace scorpion
