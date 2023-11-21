#include "MPMCQueue.h"
#include "MPSCQueue.h"
#include "SPSCQueue.h"

#include <atomic>
#include <memory>
#include <thread>
#include <vector>

using namespace std;
using namespace chrono;
using namespace scorpion;

constexpr const size_t kQueueSize = 1024;
constexpr const size_t kProducerNum = 10;
constexpr const size_t kConsumerNum = 10;
constexpr const size_t kTestCounter = 10240;

struct TestNode {
    size_t num;
    time_point<steady_clock> ts;

    TestNode()
        : num(0){};

    explicit TestNode(size_t n)
        : num(n)
        , ts(steady_clock::now()) {}

    TestNode(const TestNode &n) noexcept {
        num = n.num;
        ts = n.ts;
    };
    TestNode &operator=(const TestNode &n) noexcept {
        if (&n == this) {
            return *this;
        }
        num = n.num;
        ts = n.ts;
        return *this;
    };
};

template <typename Queue, typename Node, size_t P, size_t C, size_t N>
class TestLockFreeQueueTemplate {
public:
    TestLockFreeQueueTemplate()
        : queue_(nullptr)
        , push_(0)
        , pop_(0)
        , prod_done_(false)
        , produces_(P)
        , consumes_(C)
        , prod_sum_(0)
        , coms_sum_(0) {}

    ~TestLockFreeQueueTemplate() = default;

public:
    void Execute(Queue queue) {
        queue_ = queue;

        for (vector<thread>::size_type idx = 0; idx < consumes_.size(); ++idx) {
            consumes_[idx] = thread(&TestLockFreeQueueTemplate::consumer, this, idx);
        }
        for (vector<thread>::size_type idx = 0; idx < produces_.size(); ++idx) {
            produces_[idx] = thread(&TestLockFreeQueueTemplate::producer, this, idx);
        }
        for (auto &t : produces_) {
            if (t.joinable()) {
                t.join();
            }
        }
        prod_done_.store(true);
        for (auto &t : consumes_) {
            if (t.joinable()) {
                t.join();
            }
        }
        printf("done: Push %lu (%lu) Pop %lu (%lu)\n", push_.load(), prod_sum_.load(), pop_.load(), coms_sum_.load());
    }

private:
    static size_t uniqueNum() {
        static atomic<size_t> increase_(1);
        return increase_.fetch_add(1);
    }

    void consumer(size_t id) {
        while (true) {
            Node node;
            if (!queue_->TryPop(node)) {
                if (prod_done_.load()) {
                    break;
                }
                this_thread::sleep_for(nanoseconds(5));
            } else {
                coms_sum_ += node.num;
                ++pop_;
                this_thread::sleep_for(nanoseconds(0));
            }
        }
        {
            Node node;
            while (queue_->TryPop(node)) {
                coms_sum_ += node.num;
                ++pop_;
            }
        }
        printf("[%lu] consumer done!\n", id);
    }

    void producer(size_t id) {
        while (true) {
            auto node = Node(uniqueNum());
            if (node.num > N) {
                break;
            }
            while (!queue_->TryPush(node)) {
                this_thread::sleep_for(nanoseconds(20));
            }
            prod_sum_ += node.num;
            ++push_;
            this_thread::sleep_for(nanoseconds(10));
        }
        printf("[%lu] producer done!\n", id);
    }

private:
    Queue queue_;

    atomic<size_t> push_;
    atomic<size_t> pop_;

    atomic<bool> prod_done_;
    vector<thread> produces_;
    vector<thread> consumes_;

    atomic<size_t> prod_sum_;
    atomic<size_t> coms_sum_;
};

template <typename Queue, typename Node, size_t P, size_t C, size_t N>
class TestLockFreeQueueBulkTemplate {
public:
    TestLockFreeQueueBulkTemplate()
        : queue_(nullptr)
        , push_(0)
        , pop_(0)
        , prod_done_(false)
        , produces_(P)
        , consumes_(C)
        , prod_sum_(0)
        , coms_sum_(0) {}

    ~TestLockFreeQueueBulkTemplate() = default;

public:
    void Execute(Queue queue) {
        queue_ = queue;

        for (vector<thread>::size_type idx = 0; idx < consumes_.size(); ++idx) {
            consumes_[idx] = thread(&TestLockFreeQueueBulkTemplate::consumer, this, idx);
        }
        for (vector<thread>::size_type idx = 0; idx < produces_.size(); ++idx) {
            produces_[idx] = thread(&TestLockFreeQueueBulkTemplate::producer, this, idx);
        }
        for (auto &t : produces_) {
            if (t.joinable()) {
                t.join();
            }
        }
        prod_done_.store(true);
        for (auto &t : consumes_) {
            if (t.joinable()) {
                t.join();
            }
        }
        printf("done: Push %lu (%lu) Pop %lu (%lu)\n", push_.load(), prod_sum_.load(), pop_.load(), coms_sum_.load());
    }

private:
    static size_t uniqueNum() {
        static atomic<size_t> increase_(1);
        return increase_.fetch_add(1);
    }

    void consumer(size_t id) {
        while (true) {
            const auto bulk = queue_->TryPopBulk();
            if (bulk.empty()) {
                if (prod_done_.load()) {
                    break;
                }
                this_thread::sleep_for(nanoseconds(20));
            } else {
                for (const auto &node : bulk) {
                    coms_sum_ += node.num;
                }
                pop_ += bulk.size();
                this_thread::sleep_for(nanoseconds(10));
            }
        }
        {
            Node node;
            while (queue_->TryPop(node)) {
                coms_sum_ += node.num;
                ++pop_;
            }
        }
        printf("[%lu] consumer done!\n", id);
    }

    void producer(size_t id) {
        while (true) {
            auto node = Node(uniqueNum());
            if (node.num > N) {
                break;
            }
            while (!queue_->TryPush(node)) {
                this_thread::sleep_for(nanoseconds(20));
            }
            prod_sum_ += node.num;
            ++push_;
            this_thread::sleep_for(nanoseconds(10));
        }
        printf("[%lu] producer done!\n", id);
    }

private:
    Queue queue_;

    atomic<size_t> push_;
    atomic<size_t> pop_;

    atomic<bool> prod_done_;
    vector<thread> produces_;
    vector<thread> consumes_;

    atomic<size_t> prod_sum_;
    atomic<size_t> coms_sum_;
};

void TestSPSC() {
    auto queue(make_shared<SPSCQueue<TestNode>>(kQueueSize));
    auto test(make_shared<TestLockFreeQueueTemplate<decltype(queue), TestNode, 1, 1, kTestCounter>>());
    test->Execute(queue);
    printf("spsc done!\n");
}

void TestMPMC() {
    auto queue(make_shared<MPMCQueue<TestNode>>(kQueueSize));
    auto test(
        make_shared<TestLockFreeQueueTemplate<decltype(queue), TestNode, kProducerNum, kConsumerNum, kTestCounter>>());
    test->Execute(queue);
    printf("mpmc done!\n");
}

void TestMPSC() {
    auto queue(make_shared<MPSCQueue<TestNode>>(kQueueSize));
    auto test(make_shared<TestLockFreeQueueTemplate<decltype(queue), TestNode, kProducerNum, 1, kTestCounter>>());
    test->Execute(queue);
    printf("mpsc done!\n");
}

void TestMPSCBulk() {
    auto queue(make_shared<MPSCQueue<TestNode>>(kQueueSize));
    auto test(make_shared<TestLockFreeQueueBulkTemplate<decltype(queue), TestNode, kProducerNum, 1, kTestCounter>>());
    test->Execute(queue);
    printf("mpsc bulk done!\n");
}

int main() {
    TestSPSC();
    TestMPMC();
    TestMPSC();
    TestMPSCBulk();
    this_thread::sleep_for(seconds(2));
    return 0;
}
