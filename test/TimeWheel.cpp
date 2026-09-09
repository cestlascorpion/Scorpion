#include <assert.h>

#include "TimeWheel.h"

using namespace Scorpion;

int main() {
    TimeWheelRaw wheel;
    int calls = 0;
    wheel.Add([&calls]() {
        ++calls;
        return 0;
    }, 10, 1);

    for (int i = 0; i < 10; ++i) {
        wheel.Tick();
    }
    assert(calls == 1);

    for (int i = 0; i < 10; ++i) {
        wheel.Tick();
    }
    assert(calls == 1);
    return 0;
}
