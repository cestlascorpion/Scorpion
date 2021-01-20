#include "UnixSocket.h"

using namespace std;
using namespace Scorpion;

int main() {
    UnixSoxClient client("/tmp/SClient.sock", UNIX_STREAM);
    UnixSoxServer server("/tmp/SServer.sock", UNIX_STREAM);
    return 0;
};