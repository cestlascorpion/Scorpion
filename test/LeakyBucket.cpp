#include "LeakyBucket.h"

#include <cstdio>

using namespace std;
using namespace chrono;
using namespace scorpion;

int main() {
    LeakyBucket bucket(10, 2);

    for (int i = 0; i < 100; ++i) {
        auto ok = bucket.grant();
        if(ok) {
            printf("%d passed\n", i);
        } else {
            printf("%d denied\n", i);
        }
    }

    return 0;
}