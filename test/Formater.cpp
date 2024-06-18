#include <iostream>

#include "Chronos.h"

using namespace std;
using namespace Scorpion;

int main() {
    auto ts = time(nullptr);

    cout << TimeHelper::GetUTCDateTime(ts) << endl;
    cout << TimeHelper::GetUTCDate(ts) << endl;
    cout << TimeHelper::GetUTCTime(ts) << endl;
    cout << TimeHelper::GetLocalDateTime(ts) << endl;
    cout << TimeHelper::GetLocalDate(ts) << endl;
    cout << TimeHelper::GetLocalTime(ts) << endl;

    cout << ts << "->" << TimeHelper::GetLocalDateTime(ts) << endl;
    cout << TimeHelper::ParseUTCDateTime(TimeHelper::GetUTCDateTime(ts)) << endl;
    cout << TimeHelper::ParseUTCDateTime("2021-01-01", "%Y-%m-%d") << endl;
    cout << TimeHelper::ParseLocalDateTime(TimeHelper::GetLocalDateTime(ts)) << endl;
    cout << TimeHelper::ParseLocalDateTime("2021-01-01", "%Y-%m-%d") << endl;

    return 0;
}