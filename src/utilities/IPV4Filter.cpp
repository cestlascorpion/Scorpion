#include "IPV4Filter.h"

#include <arpa/inet.h>

#include <fstream>

using namespace std;

namespace Scorpion {

IPCidr::IPCidr(uint32_t addr) {
    _head = addr;
    _tail = addr;
}

IPCidr::IPCidr(uint32_t head, uint32_t tail) {
    _head = head;
    _tail = tail;
}

bool IPCidr::operator==(const IPCidr &cidr) const {
    return _head == cidr._head && _tail == cidr._tail;
}

bool IPCidr::operator!=(const IPCidr &cidr) const {
    return !(*this == cidr);
}

bool IPCidr::operator>(const IPCidr &cidr) const {
    return cidr < *this;
}

bool IPCidr::operator<(const IPCidr &cidr) const {
    return _head < cidr._head || (_head == cidr._head && _tail < cidr._tail);
}

} // namespace Scorpion

namespace Scorpion {

bool IPFilter::LoadConfig(const char *file, RULE_TYPE type) {
    if (file == nullptr) {
        return false;
    }
    return LoadConfig(string(file), type);
}

bool IPFilter::LoadConfig(const string &file, RULE_TYPE type) {
    if (file.empty()) {
        printf("%s: empty file!\n", __func__);
        return false;
    }

    ifstream in;
    in.open(file, ios::in);
    if (!in.is_open()) {
        printf("%s: file is not open!\n", __func__);
        return false;
    }

    string cidr;
    while (getline(in, cidr)) {
        Add(cidr, type);
    }

    in.close();
    return true;
}

bool IPFilter::Add(const char *rule, RULE_TYPE type) {
    if (rule == nullptr) {
        return false;
    }
    return Add(string(rule), type);
}

bool IPFilter::Add(const string &rule, RULE_TYPE type) {
    if (rule.empty()) {
        printf("%s: empty cidr!\n", __func__);
        return false;
    }

    uint32_t from{};
    uint32_t to{};
    if (!parseRule(rule, from, to)) {
        printf("%s: parse cidr failed! %s\n", __func__, rule.c_str());
        return false;
    }

    set<IPCidr> *plist;
    switch (type) {
    case BANNED:
        plist = &_black;
        break;
    case EXCEPTION:
        plist = &_white;
        break;
    default:
        printf("unknown type!\n");
        return false;
    }

    IPCidr new_cidr(from, to);
    for (auto it = plist->begin(); it != plist->end();) {
        if (to < it->_head || from > it->_tail) {
            ++it;
        } else if (from <= it->_head && to >= it->_tail) {
            it = plist->erase(it);
        } else {
            return false;
        }
    }
    return plist->insert(new_cidr).second;
}

bool IPFilter::Erase(const char *rule, RULE_TYPE type) {
    if (rule == nullptr) {
        return false;
    }
    return Erase(string(rule), type);
}

bool IPFilter::Erase(const string &rule, RULE_TYPE type) {
    if (rule.empty()) {
        printf("%s: empty file!\n", __func__);
        return false;
    }

    uint32_t from{};
    uint32_t to{};
    if (!parseRule(rule, from, to)) {
        printf("%s: parse cidr failed! %s\n", __func__, rule.c_str());
        return false;
    }

    set<IPCidr> *plist;
    switch (type) {
    case BANNED:
        plist = &_black;
        break;
    case EXCEPTION:
        plist = &_white;
        break;
    default:
        printf("unknown type!\n");
        return false;
    }

    IPCidr new_cidr(from, to);
    auto it = plist->find(new_cidr);
    if (it == plist->end()) {
        return false;
    }

    while (it != plist->end()) {
        if ((from == it->_head && to == it->_tail) || (from < it->_head) || (to > it->_tail)) {
            printf("%s: find a rule which has smaller/equal range, erase it!\n", __func__);
            printf("the rule: <%08X - %08X>  is bigger than <%08X - %08X>\n", from, to, it->_head, it->_tail);
            plist->erase(it);
            it = plist->find(new_cidr);
        } else {
            printf("%s: find a rule which has bigger range!\n", __func__);
            printf("the rule: <%08X - %08X>  is smaller with <%08X - %08X>\n", from, to, it->_head, it->_tail);
            return false;
        }
    }

    return true;
}

void IPFilter::Clear(RULE_TYPE type) {
    if (type == BANNED) {
        _black.clear();
    }

    if (type == EXCEPTION) {
        _white.clear();
    }
}

bool IPFilter::IsBlocked(uint32_t addr) const {
    uint32_t ad = ntohl(addr);

    IPCidr cidr(ad, ad);

    const auto contains = [&cidr](const set<IPCidr> &rules) {
        for (const auto &rule : rules) {
            if (rule._head <= cidr._head && cidr._head <= rule._tail) return true;
        }
        return false;
    };
    return contains(_black) && !contains(_white);
}

bool IPFilter::IsBlocked(const char *addr) const {
    if (addr == nullptr) {
        return false;
    }
    sockaddr_in ad;
    if (inet_pton(AF_INET, addr, &ad.sin_addr) != 1) {
        printf("%s: inet_pton() failed!\n", __func__);
        return false;
    }

    return IsBlocked(ad.sin_addr.s_addr);
}

bool IPFilter::IsBlocked(const string &addr) const {
    if (addr.empty()) {
        printf("%s: empty point addr!\n", __func__);
        return false;
    }

    return IsBlocked(addr.c_str());
}

bool IPFilter::parseRule(const string &rule, uint32_t &from, uint32_t &to) {
    if (rule.empty()) {
        printf("%s: empty rule!\n", __func__);
        return false;
    }

    if (rule.find('/') == string::npos) {
        sockaddr_in addr;
        if (inet_pton(AF_INET, rule.c_str(), &addr.sin_addr) != 1) {
            printf("%s: inet_pton() failed!\n", __func__);
            return false;
        }
        from = to = ntohl(addr.sin_addr.s_addr);
    } else {
        auto idx = rule.find('/');
        sockaddr_in addr;
        if (inet_pton(AF_INET, rule.substr(0, idx).c_str(), &addr.sin_addr) != 1) {
            printf("%s: inet_pton() failed!\n", __func__);
            return false;
        }

        auto value = rule.substr(idx + 1);
        char *end = nullptr;
        auto length = (uint32_t)strtoul(value.c_str(), &end, 10);
        if (end == value.c_str() || *end != '\0') return false;
        if (length > 32) {
            return false;
        }
        uint32_t mask = length == 0 ? 0 : (0xFFFFFFFFu << (32 - length));
        from = ntohl(addr.sin_addr.s_addr) & mask;
        to = from | ~mask;
    }

    printf("%s: head: %08X tail: %08X\n", __func__, from, to);
    return true;
}

void IPFilter::Dump(bool print) const {
    printf("info: \nblack list size %lu white list size %lu\n", _black.size(), _white.size());
    if (!print)
        return;

    printf("black rules:\n");
    for (auto &item : _black) {
        in_addr temp;
        temp.s_addr = htonl(item._head);
        string from = inet_ntoa(temp);
        temp.s_addr = htonl(item._tail);
        string to = inet_ntoa(temp);
        printf("<%08X - %08X>  ->  <%s - %s>\n", item._head, item._tail, from.c_str(), to.c_str());
    }

    printf("white rules:\n");
    for (auto &item : _white) {
        in_addr temp;
        temp.s_addr = htonl(item._head);
        string from = inet_ntoa(temp);
        temp.s_addr = htonl(item._tail);
        string to = inet_ntoa(temp);
        printf("<%08X - %08X>  ->   <%s - %s>\n", item._head, item._tail, from.c_str(), to.c_str());
    }
}

} // namespace Scorpion
