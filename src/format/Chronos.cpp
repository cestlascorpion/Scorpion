#include "Chronos.h"

using namespace std;
using namespace Scorpion;

string TimeHelper::GetUTCDateTime(time_t ts) {
    auto tm = gmtime(&ts);
    char buf[32];
    strftime(buf, 32, "%Y-%m-%d %H:%M:%S", tm);
    return string(buf);
}

string TimeHelper::GetUTCDate(time_t ts) {
    auto tm = gmtime(&ts);
    char buf[32];
    strftime(buf, 32, "%Y-%m-%d", tm);
    return string(buf);
}

string TimeHelper::GetUTCTime(time_t ts) {
    auto tm = gmtime(&ts);
    char buf[32];
    strftime(buf, 32, "%H:%M:%S", tm);
    return string(buf);
}

string TimeHelper::GetLocalDateTime(time_t ts) {
    auto tm = localtime(&ts);
    char buf[32];
    strftime(buf, 32, "%Y-%m-%d %H:%M:%S", tm);
    return string(buf);
}

string TimeHelper::GetLocalDate(time_t ts) {
    auto tm = localtime(&ts);
    char buf[32];
    strftime(buf, 32, "%Y-%m-%d", tm);
    return string(buf);
}

string TimeHelper::GetLocalTime(time_t ts) {
    auto tm = localtime(&ts);
    char buf[32];
    strftime(buf, 32, "%H:%M:%S", tm);
    return string(buf);
}

time_t TimeHelper::ParseUTCDateTime(const string &dt, const string &fmt) {
    tm tm{};
    strptime(dt.c_str(), fmt.c_str(), &tm);
    return timegm(&tm);
}

time_t TimeHelper::ParseLocalDateTime(const string &dt, const string &fmt) {
    tm tm{};
    strptime(dt.c_str(), fmt.c_str(), &tm);
    return mktime(&tm);
}
