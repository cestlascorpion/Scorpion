#include "ThreadPool.h"

#include <cstdio>

using namespace std;
using namespace scorpion;

int main() {
    ThreadPool tp(10);

    for (int id = 0; id < 1024; id++) {
        auto f = [=]() {
            printf(" %p -> %d\n", &id, id);
            return 0;
        };
        auto ret = tp.Push(f);
        if (!ret) {
            printf("push failed %d\n", id);
        }
    }
}