#include "AsyncTaskPool.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

using namespace std;
using namespace chrono;
using namespace scorpion;

constexpr const unsigned kPoolSize = 10;
constexpr const unsigned kQueueLength = 1024;
constexpr const unsigned kSleepMs = 10;
constexpr const unsigned kTimeoutMs = 100;
constexpr const size_t kProducerNum = 10;
constexpr const size_t kTestCounter = 10240;

template <typename Manager, typename Task, unsigned P, unsigned Q, unsigned S, unsigned T>
class TestAsyncTaskPoolTemplate {
public:
    static void TestExample(unsigned producer, unsigned turn, Manager *manager) {
        manager->Init(P, Q, S, T);

        vector<thread> producers;
        for (unsigned id = 0; id < producer; ++id) {
            producers.emplace_back(submitTask, turn, manager);
        }
        printf("produce done\n");
        for (auto &t : producers) {
            if (t.joinable()) {
                t.join();
            }
        }
        printf("produce exit\n");
    }

private:
    static unsigned uniqueNum() {
        static atomic<unsigned> counter(1);
        return counter.fetch_add(1);
    }

    static void submitTask(unsigned turn, Manager *manager) {
        while (true) {
            auto id = uniqueNum();
            if (id > turn) {
                break;
            }
            auto func = [id]() -> int {
                printf("==== task %u ====\n", id);
                return 0;
            };
            manager->Submit(id, Task(id, steady_clock::now(), move(func)));
            this_thread::sleep_for(milliseconds(turn % 3));
        }
    }
};

void TestMPMCManager() {
    unique_ptr<Manager<Task, MPMCQueue<Task>>> manager(new Manager<Task, MPMCQueue<Task>>);
    TestAsyncTaskPoolTemplate<Manager<Task, MPMCQueue<Task>>, Task, kPoolSize, kQueueLength, kSleepMs,
                              kTimeoutMs>::TestExample(kProducerNum, kTestCounter, manager.get());
    this_thread::sleep_for(seconds(5));
    printf("mpmc mgr done!\n");
}

void TestMPSCManager() {
    unique_ptr<Manager<Task, MPSCQueue<Task>>> manager(new Manager<Task, MPSCQueue<Task>>);
    TestAsyncTaskPoolTemplate<Manager<Task, MPSCQueue<Task>>, Task, kPoolSize, kQueueLength, kSleepMs,
                              kTimeoutMs>::TestExample(kProducerNum, kTestCounter, manager.get());
    this_thread::sleep_for(seconds(5));
    printf("mpsc mgr done!\n");
}

int main() {
    TestMPMCManager();
    TestMPSCManager();
    this_thread::sleep_for(seconds(2));
    return 0;
}
