#include "NetHelper.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>

using namespace std;
using namespace Scorpion;

string NetHelper::GetPrimaryIp() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        return {};
    }
    const char *kGoogleDnsIp = "8.8.8.8";
    uint16_t kDnsPort = 53;
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(kGoogleDnsIp);
    serv.sin_port = htons(kDnsPort);
    int err = connect(sock, (const sockaddr *)&serv, sizeof(serv));
    if (err == -1) {
        close(sock);
        return {};
    }
    sockaddr_in name;
    socklen_t namelen = sizeof(name);
    err = getsockname(sock, (sockaddr *)&name, &namelen);
    if (err == -1) {
        close(sock);
        return {};
    }

    char buffer[16];
    socklen_t buflen = sizeof(buffer);
    const char *p = inet_ntop(AF_INET, &name.sin_addr, buffer, buflen);
    if (p == nullptr) {
        close(sock);
        return {};
    }

    close(sock);
    return string(buffer);
}