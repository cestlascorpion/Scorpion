/**
 * A ipv4 address blacklist tool.
 *
 * configure file: each line is a rule
 * single ip: eg 10.120.192.19/32 or 10.120.192.19
 * ranged ip: eg 10.120.192.0/24, which is (10.120.192.0 ~ 10.120.192.255)
 *
 */

#pragma once

#include <cstdint>
#include <set>
#include <string>

namespace scorpion {

class IPCidr {
public:
    explicit IPCidr(uint32_t addr);
    IPCidr(uint32_t head, uint32_t tail);

public:
    bool operator==(const IPCidr &cidr) const;
    bool operator!=(const IPCidr &cidr) const;
    bool operator>(const IPCidr &cidr) const;
    bool operator<(const IPCidr &cidr) const;

public:
    uint32_t _head;
    uint32_t _tail;
};

class IPFilter {
public:
    enum RULE_TYPE {
        BANNED = 0,
        EXCEPTION = 1,
    };

public:
    bool LoadConfig(const char *file, RULE_TYPE type);
    bool LoadConfig(const std::string &file, RULE_TYPE type);

    bool Add(const char *rule, RULE_TYPE type);
    bool Add(const std::string &rule, RULE_TYPE type);

    bool Erase(const char *rule, RULE_TYPE type);
    bool Erase(const std::string &rule, RULE_TYPE type);

    void Clear(RULE_TYPE type);

    bool IsBlocked(uint32_t addr) const;
    bool IsBlocked(const char *addr) const;
    bool IsBlocked(const std::string &addr) const;

    void Dump(bool print) const;

private:
    static bool parseRule(const std::string &rule, uint32_t &from, uint32_t &to);

private:
    std::set<IPCidr> _black;
    std::set<IPCidr> _white;
};

} // namespace scorpion
