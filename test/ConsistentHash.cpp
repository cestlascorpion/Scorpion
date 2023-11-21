#include "ConsistentHash.h"

#include <iostream>
#include <unordered_map>

using namespace std;
using namespace scorpion;

void testInteger() {
    unordered_map<size_t, uint32_t> hosts{{101, 0}, {102, 0}, {103, 0}, {104, 0},  {105, 0},  {106, 0},
                                          {107, 0}, {108, 0}, {109, 0}, {1010, 0}, {1011, 0}, {1012, 0}};
    unique_ptr<ConsistentHash<size_t, size_t>> hash(new ConsistentHash<size_t, size_t>());
    for (const auto &pair : hosts) {
        hash->Add(pair.first, pair.first, 512);
    }

    hash->Add(1013, 1013, 512);
    hash->Del(1013, 1013);

    int64_t cost = 0;
    int64_t count = 0;
    timespec begin, end;
    for (int i = 0; i < 1000000; ++i) {
        size_t host;
        size_t url = (size_t)i;
        clock_gettime(CLOCK_MONOTONIC, &begin);
        hash->Get(url, host);
        clock_gettime(CLOCK_MONOTONIC, &end);
        ++count;
        cost += (end.tv_nsec - begin.tv_nsec) + (end.tv_sec - begin.tv_sec) * 1000000000;
        ++hosts[host];
    }
    for (const auto &pair : hosts) {
        cout << pair.first << ": " << pair.second << endl;
    }
    cout << "avg cost: " << cost / count << endl;
}

void testString() {
    unordered_map<string, uint32_t> hosts{{"ATS_1", 0}, {"ATS_2", 0},  {"ATS_3", 0},  {"ATS_4", 0},
                                          {"ATS_5", 0}, {"ATS_6", 0},  {"ATS_7", 0},  {"ATS_8", 0},
                                          {"ATS_9", 0}, {"ATS_10", 0}, {"ATS_11", 0}, {"ATS_12", 0}};
    unique_ptr<ConsistentHash<size_t, string>> hash(new ConsistentHash<size_t, string>());
    for (const auto &pair : hosts) {
        hash->Add(std::hash<string>()(pair.first), pair.first, 512);
    }

    hash->Add(std::hash<string>()("ATS_13"), "ATS_13", 512);
    hash->Del(std::hash<string>()("ATS_13"), "ATS_13");

    int64_t cost = 0;
    int64_t count = 0;
    timespec begin, end;
    for (int i = 0; i < 1000000; ++i) {
        string host;
        auto url = to_string(rand() ^ rand() ^ rand());
        clock_gettime(CLOCK_MONOTONIC, &begin);
        hash->Get(url, host);
        clock_gettime(CLOCK_MONOTONIC, &end);
        ++count;
        cost += (end.tv_nsec - begin.tv_nsec) + (end.tv_sec - begin.tv_sec) * 1000000000;
        ++hosts[host];
    }
    for (const auto &pair : hosts) {
        cout << pair.first << ": " << pair.second << endl;
    }
    cout << "avg cost: " << cost / count << endl;
}

int main() {
    testInteger();
    testString();
    return 0;
}