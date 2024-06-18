#include "IPV4Filter.h"

#include <sys/time.h>

#include <fstream>
#include <iostream>

using namespace std;
using namespace scorpion;

int main() {
    IPFilter filter;
    filter.LoadConfig("block.txt", IPFilter::BANNED);
    filter.LoadConfig("white.txt", IPFilter::EXCEPTION);
    filter.Erase("102.133.112.0/28", IPFilter::BANNED);
    filter.Erase("103.238.16.10", IPFilter::BANNED);
    filter.Erase("103.238.16.11", IPFilter::BANNED);
    filter.Erase("103.238.16.12/32", IPFilter::BANNED);
    filter.Dump(true);

    fstream in;
    in.open("test.txt", ios::in);
    string ip;
    int number = 0;
    timeval begin;
    gettimeofday(&begin, nullptr);
    while (getline(in, ip)) {
        ++number;
        bool res = filter.IsBlocked(ip);
        if (!res) {
            cout << ip << " -> not blocked" << endl;
        }
    }
    timeval end;
    gettimeofday(&end, nullptr);
    in.close();
    auto delta = (end.tv_sec - begin.tv_sec) * 1000000 + (end.tv_usec - begin.tv_usec);
    cout << "average " << delta << "/" << number << " = " << (delta / number) << "us" << endl;
    return 0;
}
