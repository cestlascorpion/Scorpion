
#include "NRWLock.h"

#include <csignal>
#include <sys/wait.h>
#include <unistd.h>

#define READER 40
#define WRITER 20

using namespace std;
using namespace scorpion;

void ReadFunc();
void WriteFunc();

int main() {
    auto *g_lock = NRWLockSingleton::Instance();
    g_lock->Init(true, "test");

    pid_t child[5];
    for (int &i : child) {
        i = fork();
        if (i == 0) {
            break;
        }
    }

    if (child[0] != 0 && child[1] != 0 && child[2] != 0 && child[3] != 0 && child[4] != 0) {
        waitpid(child[0], nullptr, 0);
        waitpid(child[1], nullptr, 0);
        waitpid(child[2], nullptr, 0);
        waitpid(child[3], nullptr, 0);
        waitpid(child[4], nullptr, 0);

        g_lock->Release();
    }

    if (child[0] == 0) {
        ReadFunc();
        _exit(0);
    }
    if (child[1] == 0) {
        ReadFunc();
        _exit(0);
    }
    if (child[2] == 0) {
        ReadFunc();
        _exit(0);
    }
    if (child[3] == 0) {
        ReadFunc();
        _exit(0);
    }
    if (child[4] == 0) {
        WriteFunc();
        _exit(0);
    }

    return 0;
}

void ReadFunc() {
    auto *g_lock = NRWLockSingleton::Instance();

    for (auto i = 0; i < READER; ++i) {
        g_lock->ReadLock(false);
        usleep(100);
        printf("pid %d: read func.\n", getpid());
        g_lock->ReadUnLock();
        usleep(100);
    }
}

void WriteFunc() {
    auto *g_lock = NRWLockSingleton::Instance();

    for (auto i = 0; i < WRITER; ++i) {
        g_lock->WriteLock(false);
        usleep(200);
        printf("pid %d: write func.\n", getpid());
        g_lock->WriteUnLock();
        usleep(200);
    }
}
