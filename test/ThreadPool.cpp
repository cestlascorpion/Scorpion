#include "ThreadPool.h"

using namespace std;
using namespace scorpion;

int main() {
    ThreadPool tp(10);

    for (int id = 0; id < 1024; id++) {
        auto ret = tp.Push([=]() {
            printf(" %p -> %d\n", &id, id);
            return 0;
        });
        if (!ret) {
            printf("push failed %d\n", id);
        }
    }
}