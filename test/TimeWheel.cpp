#include "TimeWheel.h"

#include <chrono>
#include <ctime>
#include <thread>

using namespace std;
using namespace chrono;
using namespace scorpion;

void test() {
    TimeWheelRaw twr;

    for (auto i = 1u; i <= 15; ++i) {
        twr.Add([=]() { printf("once id %u\n", i); }, i, 1);
    }

    twr.Add([]() { printf("loop inter 3\n"); }, 3, -1);
    twr.Add([]() { printf("loop inter 5\n"); }, 5, 2);
    twr.Dump();

    for (int i = 0; i < 30; ++i) {
        twr.Tick();
        this_thread::sleep_for(seconds(1));
        printf("-------------------\n");
    }
}

template <typename Resolution>
void test() {
    TimeWheel<Resolution> tw;
    thread t1([&]() {
        for (auto i = 1u; i <= 15; ++i) {
            tw.Add([=]() { printf("once id %u\n", i); }, i, 1);
        }
    });
    thread t2([&]() { tw.Add([]() { printf("loop inter 3\n"); }, 3, -1); });
    thread t3([&]() { tw.Add([]() { printf("loop inter 5\n"); }, 5, 2); });
    t1.join();
    t2.join();
    t3.join();

    this_thread::sleep_for(Resolution(20));
}

int main() {
    printf("===================\n");
    test();
    printf("===================\n");
    test<seconds>();
    printf("===================\n");
    test<milliseconds>();
    printf("===================\n");
    return 0;
}