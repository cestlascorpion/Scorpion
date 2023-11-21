#include "SpinLockMutex.h"

#include <mutex>
#include <thread>

using namespace std;
using namespace scorpion;

int main() {
    SpinLockMutex spLock;

    thread t1([&]() {
        {
            lock_guard<SpinLockMutex> lock(spLock);
            printf("locked\n");
            this_thread::sleep_for(chrono::seconds(1));
            printf("unlock\n");
        }
    });

    thread t2([&]() {
        {
            lock_guard<SpinLockMutex> lock(spLock);
            printf("locked\n");
            this_thread::sleep_for(chrono::seconds(1));
            printf("unlock\n");
        }
    });

    t1.join();
    t2.join();

    return 0;
}