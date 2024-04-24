/**
 * A simple wrapper of flock.
 *
 */

#pragma once

#include <fcntl.h>

namespace scorpion {

class FLock {
public:
    explicit FLock(const char *file);
    ~FLock();

    FLock(const FLock &) = delete;
    FLock &operator=(const FLock &) = delete;

public:
    int LockSh(bool no_block);
    int LockEx(bool no_block);
    int UnLockSh();
    int UnlockEx();

private:
    static inline void makeFlock(struct flock *lock, short type);

private:
    int _fd;
};

} // namespace scorpion
