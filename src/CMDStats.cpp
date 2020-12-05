#include "CMDStats.h"

#include <algorithm>
#include <arpa/inet.h>
#include <atomic>
#include <memory>
#include <mutex>
#include <netinet/in.h>
#include <unordered_map>

using namespace std;

namespace Scorpion {

string Ip2Str(uint32_t ip) {
    string res;
    sockaddr_in addr;
    addr.sin_addr.s_addr = htonl(ip);
    char str[INET_ADDRSTRLEN];
    const char *ptr = inet_ntop(AF_INET, &addr.sin_addr, str, sizeof(str));
    if (ptr != nullptr) {
        res = ptr;
    }
    return res;
}

uint32_t Ip2Uint32(const string &ip) {
    sockaddr_in addr;
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr.s_addr);
    return ntohl(addr.sin_addr.s_addr);
}

namespace CMDStat {

CMDStats::CMDStats(uint32_t _ip, uint16_t _port, uint32_t _cmd, uint32_t _0, uint32_t _1, uint32_t _2, uint32_t _3,
                   uint32_t _4, uint32_t _5, uint32_t _6, uint32_t _7, uint32_t _8, uint32_t _9)
    : ip(_ip)
    , port(_port)
    , cmd(_cmd)
    , counter{_0, _1, _2, _3, _4, _5, _6, _7, _8, _9} {}

struct CMDValue {
    CMDValue()
        : total(0u) {
        for (auto &item : table) {
            item.store(0u, memory_order_relaxed);
        }
    }

    atomic<uint32_t> total;
    array<atomic<uint32_t>, TABLE_SIZE> table;
};

struct CMDKeyHash {
    size_t operator()(const array<uint32_t, 3u> &k) const noexcept {
        return hash<uint32_t>()(get<0u>(k)) ^ hash<uint32_t>()(get<1u>(k)) ^ hash<uint32_t>()(get<2u>(k));
    }
};

struct GlobalStats {
    unordered_map<array<uint32_t, 3u>, unique_ptr<CMDValue>, CMDKeyHash> tbl;
    mutex mtx;
};

struct LocalStats {
    unordered_map<array<uint32_t, 3u>, CMDValue *, CMDKeyHash> tbl;
};

static GlobalStats &GetGlobalStats() {
    static GlobalStats global;
    return global;
}

static LocalStats &GetLocalStats() {
    thread_local static LocalStats local;
    return local;
}

static CMDValue *GetStatByAddr(uint32_t ip, uint32_t port, uint32_t cmd) {
    auto key = array<uint32_t, 3u>{ip, port, cmd};
    auto &local = GetLocalStats();
    auto iter = local.tbl.find(key);
    if (iter != local.tbl.end()) {
        return iter->second;
    }
    CMDValue *stats;
    {
        lock_guard<mutex> lk(GetGlobalStats().mtx);
        auto it = GetGlobalStats().tbl.find(key);
        if (it == GetGlobalStats().tbl.end()) {
            stats = new CMDValue();
            GetGlobalStats().tbl.emplace(key, unique_ptr<CMDValue>(stats));
        } else {
            stats = it->second.get();
        }
    }
    local.tbl.emplace(key, stats);
    return stats;
}

void CMDStatReport(uint32_t ip, uint16_t port, uint32_t cmd, uint32_t cost) {
    auto *stats = GetStatByAddr(ip, port, cmd);
    stats->total.fetch_add(1u, memory_order_relaxed);
    if (cost <= 5u) {
        stats->table[LEVEL_000_005].fetch_add(1u, memory_order_relaxed);
    } else if (cost <= 10u) {
        stats->table[LEVEL_005_010].fetch_add(1u, memory_order_relaxed);
    } else if (cost <= 20u) {
        stats->table[LEVEL_010_020].fetch_add(1u, memory_order_relaxed);
    } else if (cost <= 50u) {
        stats->table[LEVEL_020_050].fetch_add(1u, memory_order_relaxed);
    } else if (cost <= 100u) {
        stats->table[LEVEL_050_100].fetch_add(1u, memory_order_relaxed);
    } else if (cost <= 200u) {
        stats->table[LEVEL_100_200].fetch_add(1u, memory_order_relaxed);
    } else if (cost <= 500u) {
        stats->table[LEVEL_200_500].fetch_add(1u, memory_order_relaxed);
    } else if (cost <= 1000u) {
        stats->table[LEVEL_500_1KMS].fetch_add(1u, memory_order_relaxed);
    } else if (cost <= 2000u) {
        stats->table[LEVEL_1KMS_2KMS].fetch_add(1u, memory_order_relaxed);
    } else {
        stats->table[LEVEL_OTHERS].fetch_add(1u, memory_order_relaxed);
    }
}

void CollectCMDStats(vector<CMDStats> &stats) {
    lock_guard<mutex> lk(GetGlobalStats().mtx);

    for (auto &pair : GetGlobalStats().tbl) {
        if (pair.second->total.load(memory_order_relaxed) > 0u) {
            stats.emplace_back(get<0u>(pair.first),                                                    // ip
                               get<1u>(pair.first),                                                    // port
                               get<2u>(pair.first),                                                    // cmd
                               pair.second->table[LEVEL_000_005].exchange(0u, memory_order_relaxed),   // _0
                               pair.second->table[LEVEL_005_010].exchange(0u, memory_order_relaxed),   // _1
                               pair.second->table[LEVEL_010_020].exchange(0u, memory_order_relaxed),   // _2
                               pair.second->table[LEVEL_020_050].exchange(0u, memory_order_relaxed),   // _3
                               pair.second->table[LEVEL_050_100].exchange(0u, memory_order_relaxed),   // _4
                               pair.second->table[LEVEL_100_200].exchange(0u, memory_order_relaxed),   // _5
                               pair.second->table[LEVEL_200_500].exchange(0u, memory_order_relaxed),   // _6
                               pair.second->table[LEVEL_500_1KMS].exchange(0u, memory_order_relaxed),  // _7
                               pair.second->table[LEVEL_1KMS_2KMS].exchange(0u, memory_order_relaxed), // _8
                               pair.second->table[LEVEL_OTHERS].exchange(0u, memory_order_relaxed));   // _9
        }
    }
}

} // namespace CMDStat
} // namespace Scorpion