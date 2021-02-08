/**
 * A implementation of blocking Unix Domain Socket Client/Server.
 *
 */

#pragma once

#include <ctime>
#include <memory>
#include <sys/socket.h>

namespace Scorpion {

class UnixSocket {
public:
    explicit UnixSocket(const char *path);
    explicit UnixSocket(int sox);
    virtual ~UnixSocket();

    UnixSocket(const UnixSocket &) = delete;
    UnixSocket &operator=(const UnixSocket &) = delete;

public:
    int Create();
    int Destroy();
    int Send(const void *buffer, size_t length);
    int Recv(void *buffer, size_t length);

protected:
    int getSox() const;
    static ssize_t sendPacket(int sox, const void *buffer, size_t length);
    static ssize_t recvPacket(int sox, void *buffer, size_t length);
    static int setBlock(int sox, bool block);
    static int setTimeout(int sox, const timeval &tv);
    static int checkConnection(int sox);

protected:
    enum { UNIX_PATH_LIMIT = 107, SOCKET_TIMEOUT = 5 };

    int _sox;
    char _path[UNIX_PATH_LIMIT];
};

class UnixClient : public UnixSocket {
public:
    explicit UnixClient(const char *path);
    explicit UnixClient(int fd);
    ~UnixClient() override;

public:
    int Connect(const char *server);
};

class UnixServer : public UnixSocket {
public:
    enum { LISTEN_BLOCK_LIST = 5 };

public:
    explicit UnixServer(const char *path);
    ~UnixServer() override;

public:
    int Listen();
    std::unique_ptr<UnixClient> Accept();
};

} // namespace Scorpion