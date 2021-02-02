#include "UnixSocket.h"

#include <chrono>
#include <thread>

using namespace std;
using namespace Scorpion;

constexpr const unsigned num = 10;
constexpr const char *CLI = "/tmp/unix_client.sox";
constexpr const char *SVR = "/tmp/unix_server.sox";

void client() {
    auto cli = make_unique<UnixSoxClient>(CLI, SOCK_DGRAM);
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
    auto svr = make_unique<UnixSoxServer>(SVR, SOCK_DGRAM);
    if (svr->Create() != 0) {
        printf("create fail\n");
        return;
    }
    printf("create ok\n");
    this_thread::sleep_for(chrono::seconds(5));
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