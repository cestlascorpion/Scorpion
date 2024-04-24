/**
 * URL.h
 *
 */
#pragma once

#include <string>

namespace Scorpion {

class URLEncoding {
public:
    /**
     * Encodes a string using URL encoding.
     *
     * @param str The string to be encoded.
     * @return The URL encoded string.
     */
    static std::string URLEncode(const std::string &str);

    /**
     * Decodes a URL-encoded string.
     *
     * @param str The URL-encoded string to decode.
     * @return The decoded string.
     */
    static std::string URLDecode(const std::string &str);

    /**
     * Converts a character to its hexadecimal representation.
     *
     * @param c The character to convert.
     * @return The hexadecimal representation of the character.
     */
    static char to_hex(char c);

    /**
     * Converts a hexadecimal character to its corresponding decimal value.
     *
     * @param c The hexadecimal character to convert.
     * @return The decimal value of the hexadecimal character.
     */
    static char from_hex(char c);
};

} // namespace Scorpion