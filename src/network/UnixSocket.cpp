#include "UnixSocket.h"

#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace Scorpion {

UnixSocket::UnixSocket(const char *path, UNIX_TYPE type)
    : _soxFd(-1)
    , _block(true)
    , _type(type)
    , _path() {
    strncpy(_path, path, sizeof(_path));
}

UnixSocket::~UnixSocket() {
    if (_soxFd != -1) {
        close(_soxFd);
    }
    unlink(_path);
}

int UnixSocket::Create() {
    if (_soxFd != -1) {
        return -1;
    }
    _soxFd = socket(AF_UNIX, _type, 0);
    if (_soxFd == -1) {
        printf("socket err %d %s\n", errno, strerror(errno));
        return -1;
    }

    sockaddr_un addr{};
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, _path, sizeof(addr.sun_path) - 1);
    unlink(addr.sun_path);

    if (bind(_soxFd, (sockaddr *)&addr, (socklen_t)sizeof(addr)) == -1) {
        printf("bind error %d %s\n", errno, strerror(errno));
        return -1;
    }

    return 0;
}

int UnixSocket::Destroy() {
    if (_soxFd != -1) {
        close(_soxFd);
        _soxFd = -1;
    }
    unlink(_path);
    memset(_path, 0, sizeof(_path));
    return 0;
}

int UnixSocket::GetSox() const {
    return _soxFd;
}

bool UnixSocket::IsBlock() const {
    return _block;
}

int UnixSocket::SetBlock(bool block) {
    int flag = fcntl(_soxFd, F_GETFL);
    if (flag < 0) {
        printf("fcntl F_GETFL err %d %s\n", errno, strerror(errno));
        return -1;
    }
    if (block) {
        flag &= ~O_NONBLOCK;
    } else {
        flag |= O_NONBLOCK;
    }
    if (fcntl(_soxFd, F_SETFL, flag) != 0) {
        printf("fcntl F_SETFL err %d %s\n", errno, strerror(errno));
        return -1;
    }
    _block = block;
    return 0;
}

int UnixSocket::SetTimeout(const timeval &tv) {
    if (setsockopt(_soxFd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1) {
        printf("set socket recv timeout err %d %s\n", errno, strerror(errno));
        return -1;
    }
    if (setsockopt(_soxFd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) == -1) {
        printf("set socket send timeout err %d %s\n", errno, strerror(errno));
        return -1;
    }
    return 0;
}

int UnixSocket::CheckConnection() {
    // todo
    return 0;
}

UnixSoxClient::UnixSoxClient(const char *path, UNIX_TYPE type)
    : _sox(path, type) {}

UnixSoxClient::~UnixSoxClient() {
    _sox.Destroy();
}

int UnixSoxClient::Connect(const char *server) {
    if (server == nullptr) {
        return -1;
    }

    sockaddr_un addr{};
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, server, sizeof(addr.sun_path) - 1);

    bool block = _sox.IsBlock();
    if (block && _sox.SetBlock(false)) {
        printf("set nonblocking fail\n");
        return -1;
    }

    while (true) {
        if (connect(_sox.GetSox(), (sockaddr *)&addr, sizeof(addr)) == -1) {
            if (errno == EINTR) {
                continue;
            }
            if (errno == EINPROGRESS) {
                if (_sox.CheckConnection() == 0) {
                    break;
                } else {
                    printf("connect fail\n");
                    return -1;
                }
            } else {
                printf("connect err %d %s\n", errno, strerror(errno));
                return -1;
            }
        } else {
            break;
        }
    }

    if (block && _sox.SetBlock(true)) {
        printf("set blocking fail\n");
        return -1;
    }
    return 0;
}

UnixSoxServer::UnixSoxServer(const char *path, UNIX_TYPE type)
    : _sox(path, type) {}

UnixSoxServer::~UnixSoxServer() {
    _sox.Destroy();
}

int UnixSoxServer::Listen() {
    // todo
    return 0;
}
} // namespace Scorpion
