/**
 * Use CMDStatReport() to report the RPC result and use CollectCMDStats() to collet them.
 * All the functions are thread safe.
 */

#pragma once

#include <array>
#include <string>
#include <vector>

namespace Scorpion {

std::string Ip2Str(uint32_t ip);

uint32_t Ip2Uint32(const std::string &ip);

namespace CMDStat {

enum {
    LEVEL_000_005 = 0u,   // (0 ~ 5] ms
    LEVEL_005_010 = 1u,   // (5 ~ 10] ms
    LEVEL_010_020 = 2u,   // (10 ~ 20] ms
    LEVEL_020_050 = 3u,   // (20 ~ 50] ms
    LEVEL_050_100 = 4u,   // (50 ~ 100] ms
    LEVEL_100_200 = 5u,   // (100 ~ 200] ms
    LEVEL_200_500 = 6u,   // (200 ~ 500] ms
    LEVEL_500_1KMS = 7u,  // (500 ~ 1000] ms
    LEVEL_1KMS_2KMS = 8u, // (1000 ~ 2000] ms
    LEVEL_OTHERS = 9u,    // (2000 ~ ) ms
    TABLE_SIZE = 10u
};

struct CMDStats {
    CMDStats(uint32_t ip, uint16_t port, uint32_t cmd, uint32_t _0, uint32_t _1, uint32_t _2, uint32_t _3, uint32_t _4,
             uint32_t _5, uint32_t _6, uint32_t _7, uint32_t _8, uint32_t _9);

    const uint32_t ip;
    const uint16_t port;
    const uint32_t cmd;
    const std::array<uint32_t, TABLE_SIZE> counter;
};

void CMDStatReport(uint32_t ip, uint16_t port, uint32_t cmd, uint32_t cost);

void CollectCMDStats(std::vector<CMDStats> &stats);

void DumpCMDStats(const std::vector<CMDStats> &stats);

} // namespace CMDStat
} // namespace Scorpion
