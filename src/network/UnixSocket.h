#pragma once

#include <ctime>
#include <sys/socket.h>

namespace Scorpion {

enum UNIX_TYPE { UNIX_STREAM = SOCK_STREAM, UNIX_DGRAM = SOCK_DGRAM };

class UnixSocket {
public:
    explicit UnixSocket(const char *path, UNIX_TYPE type);
    virtual ~UnixSocket();

    UnixSocket(const UnixSocket &) = delete;
    UnixSocket &operator=(const UnixSocket &) = delete;

public:
    int Create();
    int Destroy();

public:
    int GetSox() const;
    bool IsBlock() const;
    int SetBlock(bool block);
    int SetTimeout(const timeval &tv);
    int CheckConnection();

private:
    enum { UNIX_PATH_LIMIT = 107, CONNECT_TIMEOUT = 5 };
    int _soxFd;
    bool _block;
    UNIX_TYPE _type;
    char _path[UNIX_PATH_LIMIT];
};

class UnixSoxClient {
public:
    explicit UnixSoxClient(const char *path, UNIX_TYPE type);
    ~UnixSoxClient();

public:
    int Connect(const char *server);

private:
    UnixSocket _sox;
};

class UnixSoxServer {
public:
    explicit UnixSoxServer(const char *path, UNIX_TYPE type);
    ~UnixSoxServer();

public:
    int Listen();

private:
    UnixSocket _sox;
};

} // namespace Scorpion