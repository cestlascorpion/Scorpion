#include "NRWLock.h"

#include <unistd.h>

namespace scorpion {

NRWLock::NRWLock()
    : _use_double(true)
    , _lock_name("")
    , _extra_name("")
    , _lock(nullptr)
    , _extra(nullptr) {}

NRWLock::~NRWLock() {
    Release();
}

void NRWLock::Init(bool use_double, const char *path) {
    _lock_name = path;
    _lock_name += ".lock";
    _lock = new FLock(_lock_name.c_str());

    if (use_double) {
        _extra_name = _lock_name + ".extra";
        _extra = new FLock(_extra_name.c_str());
    }

    _use_double = use_double;
}

void NRWLock::Release() {
    if (_lock != nullptr) {
        delete _lock;
        _lock = nullptr;
        unlink(_lock_name.c_str());
    }
    if (_extra != nullptr) {
        delete _extra;
        _extra = nullptr;
        unlink(_extra_name.c_str());
    }
}

bool NRWLock::ReadLock(bool no_block) {
    int ret;

    ret = _lock->LockSh(no_block);
    if (ret != 0) {
        return false;
    }

    if (!_use_double) {
        return true;
    }

    ret = _extra->LockSh(no_block);
    if (ret != 0) {
        return false;
    }

    ret = _lock->UnLockSh();
    return ret == 0;
}

bool NRWLock::WriteLock(bool no_block) {
    int ret;

    ret = _lock->LockEx(no_block);
    if (ret != 0) {
        return false;
    }

    if (!_use_double) {
        return true;
    }

    ret = _extra->LockEx(no_block);
    if (ret != 0) {
        return false;
    }

    ret = _extra->UnlockEx();
    return ret == 0;
}

bool NRWLock::ReadUnLock() {
    int ret;

    if (!_use_double) {
        ret = _lock->UnLockSh();
    } else {
        ret = _extra->UnLockSh();
    }

    return ret == 0;
}

bool NRWLock::WriteUnLock() {
    int ret;

    ret = _lock->UnlockEx();
    return ret == 0;
}

} // namespace scorpion