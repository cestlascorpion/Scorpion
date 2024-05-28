#include "UnixSocket.h"

#include <cstdio>
#include <cstring>
#include <thread>

using namespace std;
using namespace Scorpion;

constexpr const char *CLI = "/tmp/unix_client.sox";
constexpr const char *SVR = "/tmp/unix_server.sox";
constexpr const unsigned int LEN = 16;

void EchoClient() {
    auto cli = make_unique<UnixClient>(CLI);
    if (cli->Create() != 0) {
        printf("[%s] create fail\n", __func__);
        return;
    }
    printf("[%s] create ok\n", __func__);
    if (cli->Connect(SVR) != 0) {
        printf("[%s] connect fail\n", __func__);
        return;
    }
    printf("[%s] connect ok\n", __func__);

    char buffer[LEN];
    for (auto i = 0u; i < 5; ++i) {
        memset(buffer, 'A', LEN);
        int ret = cli->Send(buffer, LEN);
        if (ret != LEN) {
            printf("[%s] send fail\n", __func__);
            break;
        }
        printf("[%s] send message: %s\n", __func__, buffer);
        memset(buffer, 0, LEN);
        ret = cli->Recv(buffer, LEN);
        if (ret != LEN) {
            printf("[%s] recv fail\n", __func__);
            break;
        }
        printf("[%s] recv message: %s\n", __func__, buffer);
    }
    cli->Destroy();
    printf("[%s] close client\n", __func__);
}

void EchoServer() {
    auto svr = make_unique<UnixServer>(SVR);
    if (svr->Create() != 0) {
        printf("[%s] create fail\n", __func__);
        return;
    }
    printf("[%s] create ok\n", __func__);
    if (svr->Listen() != 0) {
        printf("[%s] listen fail\n", __func__);
        return;
    }
    printf("[%s] listen ok\n", __func__);
    auto cli = svr->Accept();
    if (cli == nullptr) {
        printf("[%s] accept fail\n", __func__);
        return;
    }
    printf("[%s] accept ok\n", __func__);

    thread t([&] {
        char buffer[LEN];
        while (true) {
            memset(buffer, 0, LEN);
            int ret = cli->Recv(buffer, LEN);
            if (ret != LEN) {
                printf("[EchoServer] recv fail\n");
                break;
            }
            printf("[EchoServer] recv message: %s\n", buffer);
            // timeout
            // this_thread::sleep_for(chrono::seconds(10));
            ret = cli->Send(buffer, LEN);
            if (ret != LEN) {
                printf("[EchoServer] recv fail\n");
                break;
            }
            printf("[EchoServer] send message: %s\n", buffer);
        }
    });
    t.join();
    printf("[%s] close accepted client\n", __func__);

    svr->Destroy();
    printf("[%s] close server\n", __func__);
}

int main() {
    thread t1(EchoServer);
    thread t2(EchoClient);
    t1.join();
    t2.join();

    return 0;
};