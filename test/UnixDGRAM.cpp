#include "UnixSocket.h"

using namespace std;
using namespace Scorpion;

int main() {
    UnixSoxClient client("/tmp/DClient.sock", UNIX_DGRAM);
    UnixSoxServer Server("/tmp/DServer.sock", UNIX_DGRAM);
    return 0;
};