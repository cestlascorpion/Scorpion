#include "UnixSocket.h"

#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace Scorpion {

static int Poll(int soxFd, short expect, short *result, int timeout) {
    pollfd poller{};
    memset(&poller, 0, sizeof(poller));
    poller.fd = soxFd;
    poller.events = expect;

    int ret = poll(&poller, 1, timeout);
    if (ret == 0) {
        errno = ETIMEDOUT;
    }
    if (ret != 0) {
        *result = poller.revents;
    }
    return ret;
}

UnixSocket::UnixSocket(const char *path, int type)
    : _sox(-1)
    , _type(type)
    , _path() {
    strncpy(_path, path, sizeof(_path));
}

UnixSocket::UnixSocket(int fd, int type)
    : _sox(fd)
    , _type(type)
    , _path() {}

UnixSocket::~UnixSocket() {
    if (_sox != -1) {
        close(_sox);
    }
    if (_path[0] != 0) {
        unlink(_path);
    }
}

int UnixSocket::Create() {
    if (_sox != -1) {
        return -1;
    }
    _sox = socket(AF_UNIX, _type, 0);
    if (_sox == -1) {
        printf("socket err %d %s\n", errno, strerror(errno));
        return -1;
    }

    sockaddr_un addr{};
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, _path, sizeof(addr.sun_path) - 1);
    unlink(addr.sun_path);

    if (bind(_sox, (sockaddr *)&addr, (socklen_t)sizeof(addr)) == -1) {
        printf("bind error %d %s\n", errno, strerror(errno));
        return -1;
    }
    return 0;
}

int UnixSocket::Destroy() {
    if (_sox != -1) {
        close(_sox);
        _sox = -1;
    }
    if (_path[0] != 0) {
        unlink(_path);
    }
    memset(_path, 0, sizeof(_path));
    return 0;
}

int UnixSocket::getSox() const {
    return _sox;
}

int UnixSocket::getType() const {
    return _type;
}

int UnixSocket::setBlock(int sox, bool block) {
    int flag = fcntl(sox, F_GETFL);
    if (flag < 0) {
        printf("fcntl F_GETFL err %d %s\n", errno, strerror(errno));
        return -1;
    }
    if (block) {
        flag &= ~O_NONBLOCK;
    } else {
        flag |= O_NONBLOCK;
    }
    if (fcntl(sox, F_SETFL, flag) != 0) {
        printf("fcntl F_SETFL err %d %s\n", errno, strerror(errno));
        return -1;
    }
    return 0;
}

int UnixSocket::setTimeout(int sox, const timeval &tv) {
    if (setsockopt(sox, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1) {
        printf("set socket recv timeout err %d %s\n", errno, strerror(errno));
        return -1;
    }
    if (setsockopt(sox, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) == -1) {
        printf("set socket send timeout err %d %s\n", errno, strerror(errno));
        return -1;
    }
    return 0;
}

int UnixSocket::checkConnection(int sox) {
    auto timeout = SOCKET_TIMEOUT * 1000;
    for (int retry = 0; retry < timeout / 1000; ++retry) {
        short expect = POLLOUT;
        short result = 0;
        int ret = Poll(sox, expect, &result, timeout);
        if (ret <= 0) {
            if (errno == EINTR) {
                continue;
            } else {
                printf("connect err %d %s\n", errno, strerror(errno));
                return -1;
            }
        }
        if ((result & expect) == 0) {
            printf("unexpect event %d expect %d\n", result, expect);
            return -1;
        }
        while (true) {
            int opt = 0;
            socklen_t len = sizeof(opt);
            if (getsockopt(sox, SOL_SOCKET, SO_ERROR, (void *)&opt, &len) == -1) {
                if (errno == EINTR) {
                    continue;
                } else {
                    printf("get socket opt err %d %s\n", errno, strerror(errno));
                    return -1;
                }
            }
            if (opt != 0) {
                printf("get socket opt err %d %s\n", errno, strerror(errno));
                return -1;
            } else {
                return 0;
            }
        }
    }
    return -1;
}

ssize_t UnixSocket::Send(int sox, const void *buffer, size_t length) {
    const char *cursor = (const char *)buffer;
    auto left = (ssize_t)length;
    while (left > 0) {
        auto byte = send(sox, cursor, (size_t)left, 0);
        if (byte > 0) {
            left -= byte;
            cursor += byte;
            continue;
        }
        if (errno == EINTR) {
            continue;
        } else {
            printf("write err %d %s\n", errno, strerror(errno));
            return -1;
        }
    }
    return (ssize_t)length - left;
}

ssize_t UnixSocket::Recv(int sox, void *buffer, size_t length, bool wait) {
    // todo MSG_TRUNC
    char *cursor = (char *)buffer;
    auto left = (ssize_t)length;
    while (left > 0) {
        auto byte = recv(sox, cursor, (size_t)left, 0);
        if (byte > 0) {
            left -= byte;
            cursor += byte;
            continue;
        }
        if (byte == 0) {
            printf("closed by peer\n");
            return -1;
        }
        if (errno == EINTR) {
            continue;
        } else {
            printf("read err %d %s\n", errno, strerror(errno));
            return -1;
        }
    }
    return (ssize_t)length - left;
}

UnixSoxClient::UnixSoxClient(const char *path, int type)
    : UnixSocket(path, type) {}

UnixSoxClient::UnixSoxClient(int fd, int type)
    : UnixSocket(fd, type) {}

UnixSoxClient::~UnixSoxClient() = default;

int UnixSoxClient::Connect(const char *server) {
    if (server == nullptr) {
        return -1;
    }
    if (getSox() == -1) {
        printf("create a socket first\n");
        return -1;
    }

    sockaddr_un addr{};
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, server, sizeof(addr.sun_path) - 1);

    if (setBlock(getSox(), false)) {
        printf("set nonblocking fail\n");
        return -1;
    }

    while (true) {
        if (connect(getSox(), (sockaddr *)&addr, sizeof(addr)) == -1) {
            if (errno == EINTR) {
                continue;
            }
            if (errno == EINPROGRESS) {
                if (checkConnection(getSox()) == 0) {
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

    if (setTimeout(getSox(), timeval{SOCKET_TIMEOUT, 0}) != 0) {
        printf("set time out fail\n");
        return -1;
    }

    if (setBlock(getSox(), true)) {
        printf("set blocking fail\n");
        return -1;
    }
    return 0;
}

UnixSoxServer::UnixSoxServer(const char *path, int type)
    : UnixSocket(path, type) {}

UnixSoxServer::~UnixSoxServer() = default;

int UnixSoxServer::Listen() {
    if (getType() == SOCK_DGRAM) {
        printf("no need to call Listen() for datagram type\n");
        return -1;
    }
    if (getSox() == -1) {
        printf("create a socket first\n");
        return -1;
    }
    if (listen(getSox(), LISTEN_BLOCK_LIST) == -1) {
        printf("listen err %d %s\n", errno, strerror(errno));
        return -1;
    }
    return 0;
}

std::unique_ptr<UnixSoxClient> UnixSoxServer::Accept() {
    if (getType() == SOCK_DGRAM) {
        printf("no need to call Accept() for datagram type\n");
        return nullptr;
    }
    if (getSox() == -1) {
        printf("create a socket first\n");
        return nullptr;
    }
    while (true) {
        sockaddr_un client{};
        socklen_t socklen = sizeof(client);
        memset(&client, 0, sizeof(client));
        client.sun_family = AF_UNIX;

        int ret = accept(getSox(), (sockaddr *)&client, &socklen);
        if (ret == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                printf("accept err %d %s\n", errno, strerror(errno));
                break;
            }
        } else {
            return std::make_unique<UnixSoxClient>(ret, _type);
        }
    }
    return nullptr;
}

SocketBuffer::SocketBuffer(size_t size)
    : _size(size)
    , _buffer(new char[size]) {}

SocketBuffer::~SocketBuffer() =default;

void SocketBuffer::Check(size_t size) {
    if (size > _size) {
        auto blocks = (size + BUFFER_SIZE - 1) / BUFFER_SIZE;
        _buffer.reset(new char[blocks * BUFFER_SIZE]);
        _size = blocks * BUFFER_SIZE;
    }
}

} // namespace Scorpion
