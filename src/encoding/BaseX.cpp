#include "BaseX.h"

using namespace std;
using namespace Scorpion;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"

string BaseEncoding::Base16EncodeWithUpperCase(const string &str) {
    static const char base16[] = "0123456789ABCDEF";

    size_t ilength = str.length();
    size_t olength = ilength * 2;

    string result;
    result.resize(olength, 0);

    for (size_t i = 0, j = 0; i < ilength;) {
        uint8_t ch = (uint8_t)str[i++];

        result[j++] = base16[(ch & 0xF0) >> 4];
        result[j++] = base16[(ch & 0x0F) >> 0];
    }

    return result;
}

string BaseEncoding::Base16EncodeWithLowerCase(const string &str) {
    static const char base16[] = "0123456789abcdef";

    size_t ilength = str.length();
    size_t olength = ilength * 2;

    string result;
    result.resize(olength, 0);

    for (size_t i = 0, j = 0; i < ilength;) {
        uint8_t ch = (uint8_t)str[i++];

        result[j++] = base16[(ch & 0xF0) >> 4];
        result[j++] = base16[(ch & 0x0F) >> 0];
    }

    return result;
}

string BaseEncoding::Base32Encode(const string &str) {
    static const char base32[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567=";

    size_t ilength = str.length();
    size_t olength = ((ilength / 5) * 8) + ((ilength % 5) ? 8 : 0);

    string result;
    result.resize(olength, 0);

    for (size_t i = 0, j = 0; i < ilength;) {
        size_t block = ((ilength - i) < 5 ? (ilength - i) : 5);
        uint8_t n1, n2, n3, n4, n5, n6, n7, n8;
        n1 = n2 = n3 = n4 = n5 = n6 = n7 = n8 = 0;

        switch (block) {
        case 5:
            n8 = (((uint8_t)str[i + 4] & 0x1F) >> 0);
            n7 = (((uint8_t)str[i + 4] & 0xE0) >> 5);
            [[fallthrough]];
        case 4:
            n7 |= (((uint8_t)str[i + 3] & 0x03) << 3);
            n6 = (((uint8_t)str[i + 3] & 0x7C) >> 2);
            n5 = (((uint8_t)str[i + 3] & 0x80) >> 7);
            [[fallthrough]];
        case 3:
            n5 |= (((uint8_t)str[i + 2] & 0x0F) << 1);
            n4 = (((uint8_t)str[i + 2] & 0xF0) >> 4);
            [[fallthrough]];
        case 2:
            n4 |= (((uint8_t)str[i + 1] & 0x01) << 4);
            n3 = (((uint8_t)str[i + 1] & 0x3E) >> 1);
            n2 = (((uint8_t)str[i + 1] & 0xC0) >> 6);
            [[fallthrough]];
        case 1:
            n2 |= (((uint8_t)str[i + 0] & 0x07) << 2);
            n1 = (((uint8_t)str[i + 0] & 0xF8) >> 3);
            break;
        default:
            // assert(false && "Invalid Base32 operation!");
            return {};
        }
        i += block;

        // Validate
        // assert((n1 <= 31) && "Invalid Base32 n1 value!");
        // assert((n2 <= 31) && "Invalid Base32 n2 value!");
        // assert((n3 <= 31) && "Invalid Base32 n3 value!");
        // assert((n4 <= 31) && "Invalid Base32 n4 value!");
        // assert((n5 <= 31) && "Invalid Base32 n5 value!");
        // assert((n6 <= 31) && "Invalid Base32 n6 value!");
        // assert((n7 <= 31) && "Invalid Base32 n7 value!");
        // assert((n8 <= 31) && "Invalid Base32 n8 value!");
        if ((n1 > 31) || (n2 > 31) || (n3 > 31) || (n4 > 31) || (n5 > 31) || (n6 > 31) || (n7 > 31) || (n8 > 31))
            return {};

        // Padding
        switch (block) {
        case 1:
            n3 = n4 = 32;
            [[fallthrough]];
        case 2:
            n5 = 32;
            [[fallthrough]];
        case 3:
            n6 = n7 = 32;
            [[fallthrough]];
        case 4:
            n8 = 32;
            [[fallthrough]];
        case 5:
            break;
        default:
            // assert(false && "Invalid Base32 operation!");
            return {};
        }

        // 8 outputs
        result[j++] = base32[n1];
        result[j++] = base32[n2];
        result[j++] = base32[n3];
        result[j++] = base32[n4];
        result[j++] = base32[n5];
        result[j++] = base32[n6];
        result[j++] = base32[n7];
        result[j++] = base32[n8];
    }

    return result;
}

string BaseEncoding::Base64Encode(const string &str) {
    static const char base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const size_t mods[] = {0, 2, 1};

    size_t ilength = str.length();
    size_t olength = 4 * ((ilength + 2) / 3);

    string result;
    result.resize(olength, 0);

    for (size_t i = 0, j = 0; i < ilength;) {
        uint32_t octet_a = i < ilength ? (uint8_t)str[i++] : 0;
        uint32_t octet_b = i < ilength ? (uint8_t)str[i++] : 0;
        uint32_t octet_c = i < ilength ? (uint8_t)str[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        result[j++] = base64[(triple >> 3 * 6) & 0x3F];
        result[j++] = base64[(triple >> 2 * 6) & 0x3F];
        result[j++] = base64[(triple >> 1 * 6) & 0x3F];
        result[j++] = base64[(triple >> 0 * 6) & 0x3F];
    }

    for (size_t i = 0; i < mods[ilength % 3]; ++i)
        result[result.size() - 1 - i] = '=';

    return result;
}

string BaseEncoding::Base16Decode(const string &str) {
    if (str.size() % 2 != 0) {
        return {};
    }

    string result;
    result.resize(str.size() / 2, 0);

    for (auto i = 0u; i < str.size(); i += 2) {
        char high = str[i];
        char low = str[i + 1];

        int high_val = (high >= '0' && high <= '9')
                           ? (high - '0')
                           : (high >= 'A' && high <= 'F') ? (high - 'A' + 10)
                                                          : (high >= 'a' && high <= 'f') ? (high - 'a' + 10) : -1;

        int low_val =
            (low >= '0' && low <= '9')
                ? (low - '0')
                : (low >= 'A' && low <= 'F') ? (low - 'A' + 10) : (low >= 'a' && low <= 'f') ? (low - 'a' + 10) : -1;

        if (high_val == -1 || low_val == -1) {
            return {};
        }

        result[i / 2] = char((high_val << 4) | low_val);
    }

    return result;
}

string BaseEncoding::Base32Decode(const string &str) {
    static const unsigned char base32[128] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0xFF, 0xFF,
        0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
        0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
        0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    const size_t ilength = str.length();
    if (ilength == 0 || ilength % 8 != 0) {
        return {};
    }

    size_t padding = 0;
    while (padding < ilength && str[ilength - padding - 1] == '=') {
        ++padding;
    }
    if (padding != 0 && padding != 1 && padding != 3 && padding != 4 && padding != 6) {
        return {};
    }
    for (size_t i = 0; i < ilength - padding; ++i) {
        if (str[i] == '=') {
            return {};
        }
    }

    size_t olength = ilength / 8 * 5;
    if (padding == 1) {
        --olength;
    } else if (padding == 3) {
        olength -= 2;
    } else if (padding == 4) {
        olength -= 3;
    } else if (padding == 6) {
        olength -= 4;
    }

    string result;
    result.reserve(olength);
    for (size_t i = 0; i < ilength; i += 8) {
        uint8_t value[8]{};
        for (size_t j = 0; j < 8; ++j) {
            const uint8_t ch = static_cast<uint8_t>(str[i + j]);
            if (ch == '=') {
                continue;
            }
            if (ch >= 0x80 || base32[ch] > 31) {
                return {};
            }
            value[j] = base32[ch];
        }

        const char bytes[5] = {
            static_cast<char>((value[0] << 3) | (value[1] >> 2)),
            static_cast<char>((value[1] << 6) | (value[2] << 1) | (value[3] >> 4)),
            static_cast<char>((value[3] << 4) | (value[4] >> 1)),
            static_cast<char>((value[4] << 7) | (value[5] << 2) | (value[6] >> 3)),
            static_cast<char>((value[6] << 5) | value[7]),
        };
        const size_t count = i + 8 == ilength ? olength - result.size() : 5;
        result.append(bytes, count);
    }

    return result;
}

string BaseEncoding::Base64Decode(const string &str) {
    static const unsigned char base64[256] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x3f, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
        0x3a, 0x3b, 0x3c, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
        0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24,
        0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00};

    size_t ilength = str.length();

    if (ilength == 0)
        return {};
    // assert((ilength % 4 == 0) && "Invalid Base64 string!");
    if (ilength % 4 != 0)
        return {};

    const auto valid = [](char ch) {
        return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '+' ||
               ch == '/' || ch == '=';
    };
    for (size_t i = 0; i < ilength; ++i) {
        if (!valid(str[i]) || (str[i] == '=' && i < ilength - 2) ||
            (str[i] == '=' && i + 1 < ilength && str[i + 1] != '='))
            return {};
    }

    size_t olength = ilength / 4 * 3;

    if (str[ilength - 1] == '=')
        olength--;
    if (str[ilength - 2] == '=')
        olength--;

    string result;
    result.resize(olength, 0);

    for (size_t i = 0, j = 0; i < ilength;) {
        uint32_t sextet_a = str[i] == '=' ? 0 & i++ : base64[(uint8_t)str[i++]];
        uint32_t sextet_b = str[i] == '=' ? 0 & i++ : base64[(uint8_t)str[i++]];
        uint32_t sextet_c = str[i] == '=' ? 0 & i++ : base64[(uint8_t)str[i++]];
        uint32_t sextet_d = str[i] == '=' ? 0 & i++ : base64[(uint8_t)str[i++]];

        uint32_t triple = (sextet_a << 3 * 6) + (sextet_b << 2 * 6) + (sextet_c << 1 * 6) + (sextet_d << 0 * 6);

        if (j < olength)
            result[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < olength)
            result[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < olength)
            result[j++] = (triple >> 0 * 8) & 0xFF;
    }

    return result;
}

#pragma GCC diagnostic pop
