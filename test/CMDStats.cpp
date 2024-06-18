#include "CMDStats.h"

#include <sys/wait.h>
#include <unistd.h>

#include <atomic>
#include <set>
#include <thread>
#include <vector>

using namespace std;
using namespace chrono;
using namespace Scorpion;

void func(const uint32_t kWorkerNum, const uint32_t kRunSec) {
    const vector<string> ipTable{
        "192.168.1.101", "192.168.1.102", "192.168.1.103", "192.168.1.104", "192.168.1.105", "192.168.1.106",
        "192.168.1.111", "192.168.1.112", "192.168.1.113", "192.168.1.114", "192.168.1.115", "192.168.1.116",
        "192.168.1.121", "192.168.1.122", "192.168.1.123", "192.168.1.124", "192.168.1.125", "192.168.1.126",
        "192.168.1.131", "192.168.1.132", "192.168.1.133", "192.168.1.134", "192.168.1.135", "192.168.1.136",

        "192.168.2.101", "192.168.2.102", "192.168.2.103", "192.168.2.104", "192.168.2.105", "192.168.2.106",
        "192.168.2.111", "192.168.2.112", "192.168.2.113", "192.168.2.114", "192.168.2.115", "192.168.2.116",
        "192.168.2.121", "192.168.2.122", "192.168.2.123", "192.168.2.124", "192.168.2.125", "192.168.2.126",
        "192.168.2.131", "192.168.2.132", "192.168.2.133", "192.168.2.134", "192.168.2.135", "192.168.2.136",

        "192.168.3.101", "192.168.3.102", "192.168.3.103", "192.168.3.104", "192.168.3.105", "192.168.3.106",
        "192.168.3.111", "192.168.3.112", "192.168.3.113", "192.168.3.114", "192.168.3.115", "192.168.3.116",
        "192.168.3.121", "192.168.3.122", "192.168.3.123", "192.168.3.124", "192.168.3.125", "192.168.3.126",
        "192.168.3.131", "192.168.3.132", "192.168.3.133", "192.168.3.134", "192.168.3.135", "192.168.3.136",
    };

    vector<array<uint64_t, 2u>> cost(kWorkerNum + 1u, {0, 0});
    atomic<bool> running{true};

    vector<thread> workers;
    for (auto i = 0u; i < kWorkerNum; ++i) {
        workers.emplace_back(
            [&](uint32_t id) {
                srand(id);
                timespec t1, t2;
                while (running.load(memory_order_acquire)) {
                    auto random = rand();
                    auto ip = ipTable[random % ipTable.size()];
                    auto port = random % 1024u + 1024;
                    auto cmd = random % 40u;
                    auto c = random % 2100;

                    clock_gettime(CLOCK_MONOTONIC, &t1);
                    CMDStat::CMDStatReport(Ip2Uint32(ip), uint16_t(port), cmd, c);
                    clock_gettime(CLOCK_MONOTONIC, &t2);

                    cost[id][0] += uint64_t((t2.tv_sec - t1.tv_sec) * 1000000000 + (t2.tv_nsec - t1.tv_nsec));
                    ++cost[id][1];
                    this_thread::sleep_for(milliseconds(2));
                }
            },
            i);
    }
    thread collect([&]() {
        timespec t1, t2;
        while (running.load(memory_order_acquire)) {
            this_thread::sleep_for(seconds(10));
            vector<CMDStat::CMDStats> data;
            clock_gettime(CLOCK_MONOTONIC, &t1);
            CMDStat::CollectCMDStats(data);
            clock_gettime(CLOCK_MONOTONIC, &t2);
            cost[kWorkerNum][0] += uint64_t((t2.tv_sec - t1.tv_sec) * 1000000000 + (t2.tv_nsec - t1.tv_nsec));
            ++cost[kWorkerNum][1];
        }
    });

    this_thread::sleep_for(seconds(kRunSec));
    running.store(false, memory_order_release);
    collect.join();
    for (auto &w : workers) {
        w.join();
    }

    uint64_t sum{0};
    uint64_t num{0};
    for (auto i = 0u; i < kWorkerNum; ++i) {
        sum += cost[i][0];
        num += cost[i][1];
    }
    printf("[%d] *report: %lu ns\n", getpid(), num > 0 ? sum / num : 0);
    printf("[%d] collect: %lu ns\n", getpid(), cost[kWorkerNum][1] > 0 ? cost[kWorkerNum][0] / cost[kWorkerNum][1] : 0);
}

int main(int argc, char **argv) {
    uint32_t pNum{1}, tNum{1}, tSec{120};
    if (argc != 4) {
        printf("input [process_num] [thread_num] [run_sec]\n");
        printf("use default: 1 process 1 thread 120 sec\n");
    } else {
        pNum = uint32_t(stoi(argv[1], nullptr, 10));
        tNum = uint32_t(stoi(argv[2], nullptr, 10));
        tSec = uint32_t(stoi(argv[3], nullptr, 10));
    }

    set<pid_t> set;
    for (auto i = 0u; i < pNum; ++i) {
        auto pid = fork();
        if (pid == 0) {
            break;
        } else {
            set.insert(pid);
        }
    }

    if (set.size() != pNum) {
        func(tNum, tSec);
        exit(0);
    }

    while (!set.empty()) {
        auto pid = waitpid(-1, nullptr, 0);
        if (set.find(pid) != set.end()) {
            set.erase(pid);
        }
    }
    return 0;
}