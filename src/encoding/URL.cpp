#include "URL.h"

#include <assert.h>

using namespace std;
using namespace Scorpion;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"

char URLEncoding::to_hex(char c) {
    return c < 10 ? c + '0' : c + 'A' - 10;
}

char URLEncoding::from_hex(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else {
        assert(false && "Invalid hex character");
    }
}

string URLEncoding::URLEncode(const string &str) {
    string encoded;
    for (char c : str) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += c;
        } else if (c == ' ') {
            encoded += '+';
        } else {
            encoded += '%';
            encoded += to_hex(c >> 4);
            encoded += to_hex(c & 0xf);
        }
    }
    return encoded;
}

string URLEncoding::URLDecode(const string &str) {
    string decoded;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%') {
            decoded += from_hex(str[i + 1]) << 4 | from_hex(str[i + 2]);
            i += 2;
        } else if (str[i] == '+') {
            decoded += ' ';
        } else {
            decoded += str[i];
        }
    }
    return decoded;
}

#pragma GCC diagnostic pop
