/**
 * A implementation of blocking Unix Domain Socket Client/Server.
 *
 */

#pragma once

#include <ctime>
#include <memory>
#include <sys/socket.h>

namespace Scorpion {

struct SocketBuffer {
public:
    explicit SocketBuffer(size_t size);
    ~SocketBuffer();

    SocketBuffer(const SocketBuffer &) = delete;
    SocketBuffer &operator=(const SocketBuffer &) = delete;

public:
    void Check(size_t size);

public:
    enum { BUFFER_SIZE = 4096 };

    size_t _size;
    std::unique_ptr<char[]> _buffer;
};

class UnixSocket {
public:
    explicit UnixSocket(const char *path, int type);
    explicit UnixSocket(int fd, int type);
    virtual ~UnixSocket();

    UnixSocket(const UnixSocket &) = delete;
    UnixSocket &operator=(const UnixSocket &) = delete;

public:
    int Create();
    int Destroy();
    static ssize_t Send(int sox, const void *buffer, size_t length);
    static ssize_t Recv(int sox, void *buffer, size_t length, bool wait);

protected:
    int getSox() const;
    int getType() const;

    static int setBlock(int sox, bool block);
    static int setTimeout(int sox, const timeval &tv);
    static int checkConnection(int sox);

protected:
    enum { UNIX_PATH_LIMIT = 107, SOCKET_TIMEOUT = 5 };

    int _sox;
    int _type;
    char _path[UNIX_PATH_LIMIT];
};

class UnixSoxClient : public UnixSocket {
public:
    UnixSoxClient(const char *path, int type);
    UnixSoxClient(int fd, int type);
    ~UnixSoxClient() override;

public:
    int Connect(const char *server);
};

class UnixSoxServer : public UnixSocket {
public:
    enum { LISTEN_BLOCK_LIST = 5 };

public:
    UnixSoxServer(const char *path, int type);
    ~UnixSoxServer() override;

public:
    int Listen();
    std::unique_ptr<UnixSoxClient> Accept();
};

} // namespace Scorpion