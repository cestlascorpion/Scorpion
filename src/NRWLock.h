/**
 * A novel implementation of read-write lock.
 * inspired by Yuriy Yevtukhov @stackoverflow [link](https://stackoverflow.com/questions/27625597/
 * how-to-implement-a-writer-preferring-read-write-lock-for-nix-processes).
 *
 */

#pragma once

#include "FLock.h"
#include "SingletonTemplate.h"

#include <string>

namespace scorpion {

class NRWLock {
public:
    NRWLock();
    ~NRWLock();

    NRWLock(const NRWLock &) = delete;
    NRWLock &operator=(const NRWLock &) = delete;

public:
    // use_double: use two flock or not, path: name of file to be locked
    void Init(bool use_double, const char *path);
    void Release();

    // no_block: use no blocked mode
    bool ReadLock(bool no_block);
    bool WriteLock(bool no_block);
    bool ReadUnLock();
    bool WriteUnLock();

private:
    bool _use_double;

    std::string _lock_name;
    std::string _extra_name;

    FLock *_lock;
    FLock *_extra;
};

using NRWLockSingleton = Singleton<NRWLock>;

} // namespace scorpion
