/**
 * A implementation of consistent hash algorithm.
 *
 */

#pragma once

#include <map>
#include <memory>
#include <string>
#include <unordered_map>

namespace scorpion {

template <typename VALUE>
struct Node {
    Node(const VALUE &value, uint32_t scale)
        : _value(value)
        , _scale(scale) {}

    const VALUE _value;
    const uint32_t _scale;
};

template <typename KEY, typename VALUE>
class ConsistentHash {
public:
    ConsistentHash() = default;
    ~ConsistentHash() = default;

public:
    int Add(const KEY &key, const VALUE &value, uint32_t scale);
    int Del(const KEY &key, const VALUE &value);
    int Get(const std::string &key, VALUE &value) const;
    int Get(const std::size_t &key, VALUE &value) const;

private:
    std::unordered_map<KEY, std::unique_ptr<Node<VALUE>>> _data;
    std::map<std::size_t, Node<VALUE> *> _circle;
};

template <typename KEY>
std::string inline GenVnodeKey(const KEY &key, size_t idx) {
    return std::to_string(key) + "." + std::to_string(idx);
}

template <>
std::string inline GenVnodeKey(const std::string &key, size_t idx) {
    return key + "." + std::to_string(idx);
}

template <typename KEY, typename VALUE>
int ConsistentHash<KEY, VALUE>::Add(const KEY &key, const VALUE &value, uint32_t scale) {
    auto iter = _data.find(key);
    if (iter != _data.end()) {
        return -1;
    }
    std::unique_ptr<Node<VALUE>> host(new Node<VALUE>(value, scale));
    for (auto idx = 0u; idx < host->_scale; ++idx) {
        auto vHash = std::hash<std::string>()(GenVnodeKey(key, idx));
        auto vIter = _circle.find(vHash);
        if (vIter != _circle.end()) {
            continue;
        }
        _circle.emplace(vHash, host.get());
    }
    _data.emplace(key, std::move(host));
    return 0;
}

template <typename KEY, typename VALUE>
int ConsistentHash<KEY, VALUE>::Del(const KEY &key, const VALUE &value) {
    auto iter = _data.find(key);
    if (iter == _data.end()) {
        return -1;
    }
    if (iter->second->_value != value) {
        printf("[%s] missing value\n", __func__);
        return -1;
    }
    for (auto idx = 0u; idx < iter->second->_scale; ++idx) {
        auto vHash = std::hash<std::string>()(GenVnodeKey(key, idx));
        auto vIter = _circle.find(vHash);
        if (vIter == _circle.end()) {
            printf("[%s] missing vnode\n", __func__);
            continue;
        }
        if (vIter->second != iter->second.get()) {
            printf("[%s] duplicated vnode\n", __func__);
            continue;
        }
        _circle.erase(vIter);
    }
    _data.erase(iter);
    return 0;
}

template <typename KEY, typename VALUE>
int ConsistentHash<KEY, VALUE>::Get(const std::string &key, VALUE &value) const {
    if (_circle.empty()) {
        return -1;
    }
    auto vIter = _circle.lower_bound(std::hash<std::string>()(key));
    if (vIter == _circle.end()) {
        vIter = _circle.begin();
    }
    value = vIter->second->_value;
    return 0;
}

template <typename KEY, typename VALUE>
int ConsistentHash<KEY, VALUE>::Get(const std::size_t &key, VALUE &value) const {
    if (_circle.empty()) {
        return -1;
    }
    auto vIter = _circle.lower_bound(std::hash<std::string>()(std::to_string(key)));
    if (vIter == _circle.end()) {
        vIter = _circle.begin();
    }
    value = vIter->second->_value;
    return 0;
}

} // namespace scorpion
