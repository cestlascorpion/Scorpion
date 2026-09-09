#include "UnixSocket.h"

#include <assert.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace Scorpion;

int main() {
    char path[] = "/tmp/scorpion-unix-socket-XXXXXX";
    const int fd = mkstemp(path);
    assert(fd != -1);
    assert(write(fd, "x", 1) == 1);
    assert(close(fd) == 0);

    {
        UnixServer server(path);
        assert(server.Create() == -1);
    }

    struct stat info {};
    assert(lstat(path, &info) == 0);
    assert(S_ISREG(info.st_mode));
    assert(unlink(path) == 0);

    return 0;
}
