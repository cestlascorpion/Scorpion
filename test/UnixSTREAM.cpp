#include "UnixSocket.h"

#include <chrono>
#include <thread>

using namespace std;
using namespace Scorpion;

constexpr const char *CLI = "/tmp/unix_client.sox";
constexpr const char *SVR = "/tmp/unix_server.sox";

void client() {
    auto cli = make_unique<UnixSoxClient>(CLI, SOCK_STREAM);
    if (cli->Create() != 0) {
        printf("create fail\n");
        return;
    }
    printf("create ok\n");
    if (cli->Connect(SVR) != 0) {
        printf("connect fail\n");
        return;
    }
    printf("connect ok\n");
    this_thread::sleep_for(chrono::seconds(3));
    cli->Destroy();
}

void server() {
    auto svr = make_unique<UnixSoxServer>(SVR, SOCK_STREAM);
    if (svr->Create() != 0) {
        printf("create fail\n");
        return;
    }
    printf("create ok\n");
    if (svr->Listen() != 0) {
        printf("listen fail\n");
        return;
    }
    printf("listen ok\n");
    auto client = svr->Accept();
    if (client == nullptr) {
        printf("accept fail\n");
        return;
    }
    printf("accept ok\n");
    // client->Destroy();
    if (svr->Destroy() != 0) {
        printf("destroy fail\n");
        return;
    }
}

int main() {
    thread t0(client);
    t0.join();

    thread t1(server);
    thread t2(client);
    t1.join();
    t2.join();
    return 0;
};