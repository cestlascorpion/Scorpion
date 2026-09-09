#include <assert.h>

#include <atomic>
#include <string>
#include <thread>
#include <vector>

#include "Chronos.h"

using namespace Scorpion;

int main() {
    constexpr time_t first = 0;
    constexpr time_t second = 2524608000;
    const std::string firstText = "1970-01-01 00:00:00";
    const std::string secondText = "2050-01-01 00:00:00";
    std::atomic<bool> failed{false};
    std::vector<std::thread> workers;

    for (int id = 0; id < 16; ++id) {
        workers.emplace_back([&, id]() {
            const time_t value = id % 2 == 0 ? first : second;
            const std::string &expected = id % 2 == 0 ? firstText : secondText;
            for (int i = 0; i < 20000; ++i) {
                if (TimeHelper::GetUTCDateTime(value) != expected) {
                    failed.store(true);
                    return;
                }
            }
        });
    }
    for (auto &worker : workers) {
        worker.join();
    }

    assert(!failed.load());
    return 0;
}
