#include "ThreadPool.h"

using namespace std;
using namespace scorpion;

int main() {
    // TODO: BUG HERE
    ThreadPool tp(1, 1);

    for (int i = 0; i < 1024; i++) {
        tp.Add([=]() {
            printf(" %p -> %d\n", &i, i);
            return 0;
        });
    }
}