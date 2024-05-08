#include "NetHelper.h"
#include <iostream>

using namespace std;
using namespace Scorpion;

int main() {
    auto primaryIp = NetHelper::GetPrimaryIp();
    cout << "Primary IP: " << primaryIp << endl;
    return 0;
}