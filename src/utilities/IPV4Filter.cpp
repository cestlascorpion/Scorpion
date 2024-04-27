#include "IPV4Filter.h"

#include <arpa/inet.h>
#include <fstream>

using namespace std;

namespace scorpion {

IPCidr::IPCidr(uint32_t addr) {
    _head = addr;
    _tail = addr;
}

IPCidr::IPCidr(uint32_t head, uint32_t tail) {
    _head = head;
    _tail = tail;
}

bool IPCidr::operator==(const IPCidr &cidr) const {
    if ((cidr._head >= _head) && (cidr._tail <= _tail)) {
        return true;
    }
    if ((_head >= cidr._head) && (_tail <= cidr._tail)) {
        return true;
    }
    return false;
}

bool IPCidr::operator!=(const IPCidr &cidr) const {
    if ((cidr._head >= _head) && (cidr._tail <= _tail)) {
        return false;
    }
    if ((_head >= cidr._head) && (_tail <= cidr._tail)) {
        return false;
    }
    return true;
}

bool IPCidr::operator>(const IPCidr &cidr) const {
    if (this->operator==(cidr)) {
        return false;
    }
    return _tail > cidr._tail;
}

bool IPCidr::operator<(const IPCidr &cidr) const {
    if (this->operator==(cidr)) {
        return false;
    }
    return _head < cidr._head;
}

} // namespace scorpion

namespace scorpion {

bool IPFilter::LoadConfig(const char *file, RULE_TYPE type) {
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
    auto it = plist->find(new_cidr);
    if (it == plist->end()) {
        printf("not found. add rule: <%08X - %08X>\n", from, to);
        return plist->insert(new_cidr).second;
    }

    while (it != plist->end()) {
        if (from < it->_head || to > it->_tail) {
            printf("%s: conflict with another rule which has smaller range, erase it!\n", __func__);
            printf("the rule: <%08X - %08X>  is conflict with <%08X - %08X>\n", from, to, it->_head, it->_tail);
            plist->erase(it);
            it = plist->find(new_cidr);
        } else {
            printf("%s: conflict with another rule which has bigger range!\n", __func__);
            printf("the rule: <%08X - %08X>  is conflict with <%08X - %08X>\n", from, to, it->_head, it->_tail);
            return false;
        }
    }

    return plist->insert(new_cidr).second;
}

bool IPFilter::Erase(const char *rule, RULE_TYPE type) {
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

    if (_black.find(cidr) == _black.end()) {
        return false;
    } else {
        return _white.find(cidr) == _white.end();
    }
}

bool IPFilter::IsBlocked(const char *addr) const {
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

        from = ntohl(addr.sin_addr.s_addr);
        auto length = (uint32_t)strtoul(rule.substr(idx + 1, rule.size() - idx).c_str(), nullptr, 10);
        if (length > 32) {
            return false;
        }
        if (length == 32) {
            to = from;
        } else {
            to = from | (0xFFFFFFFF >> length);
        }
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

} // namespace scorpion
