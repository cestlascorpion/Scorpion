#include "NetHelper.h"
#include <cstring>
#include <iostream>

using namespace std;
using namespace Scorpion;

int main() {
    auto primaryIp = NetHelper::GetPrimaryIp();
    cout << "Primary IP: " << primaryIp << endl;

    char buffer[32]{};
    strncpy(buffer, primaryIp.c_str() + (primaryIp.size() - 3), sizeof(buffer) - 1);
    cout << buffer << endl;
    return 0;
}