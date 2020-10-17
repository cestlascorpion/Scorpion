#include "TimeWheel.h"

#include <chrono>
#include <ctime>
#include <thread>

using namespace std;
using namespace chrono;
using namespace scorpion;

void testRaw() {
    TimeWheelRaw twr;

    for (auto i = 1u; i <= 15; ++i) {
        twr.Add([=]() { printf("[%ld] once id %u\n", time(nullptr), i); }, i, 1);
    }

    twr.Add([]() { printf("[%ld] loop inter 3\n", time(nullptr)); }, 3, -1);
    twr.Add([]() { printf("[%ld] loop inter 5\n", time(nullptr)); }, 5, 2);
    twr.Dump();

    for (int i = 0; i < 20; ++i) {
        twr.Tick();
        this_thread::sleep_for(seconds(1));
        printf("-------------------\n");
    }
}

template <typename Resolution, bool Async>
void test() {
    TimeWheel<Resolution> tw(Async);
    thread t1([&]() {
        for (auto i = 1u; i <= 15; ++i) {
            tw.Add([=]() { printf("[%ld] once id %u\n", time(nullptr), i); }, i, 1);
        }
    });
    thread t2([&]() {
        tw.Add(
            []() {
                printf("[%ld] loop inter 3 begin\n", time(nullptr));
                if (Async) {
                    this_thread::sleep_for(seconds(3));
                }
                printf("[%ld] loop inter 3 end\n", time(nullptr));
            },
            3, -1);
    });
    thread t3([&]() {
        tw.Add(
            []() {
                printf("[%ld] loop inter 5 begin\n", time(nullptr));
                if (Async) {
                    this_thread::sleep_for(seconds(5));
                }
                printf("[%ld] loop inter 5 end\n", time(nullptr));
            },
            5, 2);
    });
    t1.join();
    t2.join();
    t3.join();

    this_thread::sleep_for(Resolution(30));
}

int main() {
    printf("===================\n");
    testRaw();
    printf("===================\n");
    test<seconds, false>();
    printf("===================\n");
    test<seconds, true>();
    printf("===================\n");
    return 0;
}