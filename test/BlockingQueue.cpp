#include "BlockingQueue.h"

#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>
#include <unordered_map>
#include <vector>

using namespace std;
using namespace scorpion;

const int kMaxThreads = 16;
static_assert((kMaxThreads & (kMaxThreads - 1)) == 0, "Make sure kMaxThreads == 2^n");

int maxElements;
BlockingQueue<int> q(1024);
atomic<int> cnt(0);
atomic<bool> start(false);
unordered_map<int, int *> elements2timespan;

void onPush(int divide) {
    while (!start) {
        this_thread::yield();
    }
    for (int i = 0; i < maxElements / divide; ++i) {
        q.Push(i);
    }
}

void onPop() {
    while (!start) {
        this_thread::yield();
    }
    int x = 0;
    for (; cnt.load(memory_order_relaxed) < maxElements;) {
        if (q.TryPop(x)) {
            cnt.fetch_add(1, memory_order_relaxed);
        }
    }
}

void TestConcurrentPushAndPop() {
    vector<thread> push_threads;
    for (int i = 0; i < kMaxThreads / 2; ++i) {
        push_threads.emplace_back(onPush, kMaxThreads / 2);
    }

    vector<thread> pop_threads;
    for (int i = 0; i < kMaxThreads / 2; ++i) {
        pop_threads.emplace_back(onPop);
    }

    cnt = 0;
    auto t1_ = chrono::steady_clock::now();
    start = true;
    for (size_t i = 0; i < kMaxThreads / 2; ++i) {
        push_threads[i].join();
    }
    for (size_t i = 0; i < kMaxThreads / 2; ++i) {
        pop_threads[i].join();
    }
    auto t2_ = chrono::steady_clock::now();

    assert(static_cast<int>(q.Size()) == 0 && cnt == maxElements);
    auto ms = chrono::duration_cast<chrono::milliseconds>(t2_ - t1_).count();
    elements2timespan[maxElements][2] += (int)ms;
    cout << maxElements << " elements push and pop, timespan=" << ms << "ms"
         << "\n";

    cnt = 0;
    start = false;
}

auto onPop_with_count = [](unordered_map<int, int> &element2count) {
    while (!start) {
        this_thread::yield();
    }
    int x = 0;
    for (; cnt.load(memory_order_relaxed) < maxElements;) {
        if (q.TryPop(x)) {
            cnt.fetch_add(1, memory_order_relaxed);
            ++element2count[x];
        }
    }
};

unordered_map<int, int> element2count[kMaxThreads / 2];

void TestCorrectness() {
    maxElements = 1000000;
    assert(maxElements % kMaxThreads == 0);

    for (int i = 0; i < maxElements / kMaxThreads; ++i) {
        for (auto &j : element2count) {
            j[i] = 0;
        }
    }

    vector<thread> push_threads;
    for (int i = 0; i < kMaxThreads / 2; ++i) {
        push_threads.emplace_back(onPush, kMaxThreads / 2);
    }

    vector<thread> pop_threads;
    for (auto &i : element2count) {
        pop_threads.emplace_back(onPop_with_count, ref(i));
    }

    cnt = 0;
    start = true;
    for (size_t i = 0; i < kMaxThreads / 2; ++i) {
        push_threads[i].join();
    }
    for (size_t i = 0; i < kMaxThreads / 2; ++i) {
        pop_threads[i].join();
    }

    assert(static_cast<int>(q.Size()) == 0 && cnt == maxElements);
    for (int i = 0; i < maxElements / kMaxThreads; ++i) {
        int sum = 0;
        for (auto &j : element2count) {
            sum += j[i];
        }
        assert(sum == kMaxThreads / 2);
    }
}

int main() {
    int elements[] = {10000, 100000, 1000000};
    int timespan1[] = {0, 0, 0};
    int timespan2[] = {0, 0, 0};
    int timespan3[] = {0, 0, 0};

    elements2timespan[10000] = timespan1;
    elements2timespan[100000] = timespan2;
    elements2timespan[1000000] = timespan3;

    for (int i = 0; i < 10; ++i) {
        for (int element : elements) {
            maxElements = element;
            TestConcurrentPushAndPop();
            cout << "\n";
        }
    }

    for (int element : elements) {
        maxElements = element;
        float avg = static_cast<float>(elements2timespan[maxElements][0]) / 10.0f;
        cout << maxElements << " elements push, average timespan=" << avg << "ms"
             << "\n";
        avg = static_cast<float>(elements2timespan[maxElements][1]) / 10.0f;
        cout << maxElements << " elements pop, average timespan=" << avg << "ms"
             << "\n";
        avg = static_cast<float>(elements2timespan[maxElements][2]) / 10.0f;
        cout << maxElements << " elements push and pop, average timespan=" << avg << "ms"
             << "\n";
        cout << "\n";
    }

    TestCorrectness();

    return 0;
}
