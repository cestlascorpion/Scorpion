#include "ThreadPool.h"

using namespace std;
using namespace scorpion;

int main() {
    ThreadPool tp(3, 1024); // BUG with Ubuntu 18.04/GCC-7 ❓

    for (int i = 0; i < 1024; i++) {
        tp.Add([=]() {
            printf(" %p -> %d\n", &i, i);
            return 0;
        });
    }
}