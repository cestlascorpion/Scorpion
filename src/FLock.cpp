#include "FLock.h"

#include <sys/stat.h>
#include <unistd.h>

namespace scorpion {

FLock::FLock(const char *file)
    : _fd(-1) {
    mode_t old_mast = umask(0);
    _fd = open(file, O_CREAT | O_RDWR | O_CLOEXEC, 0666);
    umask(old_mast);
}

FLock::~FLock() {
    if (_fd >= 0) {
        close(_fd);
    }
}

int FLock::LockSh(bool no_block) {
    if (_fd < 0) {
        return -1;
    }

    struct flock flock;
    makeFlock(&flock, F_RDLCK);
    int cmd = no_block ? F_SETLK : F_SETLKW;
    if (fcntl(_fd, cmd, &flock) != 0) {
        return -1;
    }

    return 0;
}

int FLock::LockEx(bool no_block) {
    if (_fd < 0) {
        return -1;
    }

    struct flock flock;
    makeFlock(&flock, F_WRLCK);
    int cmd = no_block ? F_SETLK : F_SETLKW;
    if (fcntl(_fd, cmd, &flock) != 0) {
        return -1;
    }

    return 0;
}

int FLock::UnLockSh() {
    if (_fd < 0) {
        return -1;
    }

    struct flock flock;
    makeFlock(&flock, F_UNLCK);

    if (fcntl(_fd, F_SETLKW, &flock) != 0) {
        return -1;
    }

    return 0;
}

int FLock::UnlockEx() {
    if (_fd < 0) {
        return -1;
    }

    struct flock flock;
    makeFlock(&flock, F_UNLCK);

    if (fcntl(_fd, F_SETLKW, &flock) != 0) {
        return -1;
    }
    return 0;
}

void FLock::makeFlock(struct flock *lock, short type) {
    lock->l_type = type;
    lock->l_whence = SEEK_SET;
    lock->l_start = 0;
    lock->l_len = 0;
}

} // namespace scorpion