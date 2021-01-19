#pragma once

#include <sys/un.h>

namespace Scorpion {

class UnixBase {
public:
    UnixBase();
    virtual ~UnixBase();

public:
    int Create();
    int Destroy();
    int Send();
    int Recv();

protected:
    enum { UNIX_PATH_LIMIT = 108 };

    int _soxFd;
    char _path[UNIX_PATH_LIMIT];
};

class UnixBaseClient : public UnixBase {
public:
    UnixBaseClient();
    ~UnixBaseClient() override;

public:
    int Connect();
};

class UnixBaseServer : public UnixBase {
public:
    UnixBaseServer();
    ~UnixBaseServer() override;

public:
    int Listen();
    int Accept();
};

} // namespace Scorpion