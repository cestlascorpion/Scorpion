/**
 * BaseX.h
 *
 */
#pragma once

#include <string>

namespace Scorpion {

class BaseEncoding {
public:
    /**
     * Encodes a string using Base16 encoding.
     *
     * @param str The string to be encoded.
     * @return The Base16 encoded string(Upper-Case).
     */
    static std::string Base16EncodeWithUpperCase(const std::string &str);
    /**
     * Encodes a string using Base16 encoding.
     *
     * @param str The string to be encoded.
     * @return The Base16 encoded string(Lower-Case).
     */
    static std::string Base16EncodeWithLowerCase(const std::string &str);
    /**
     * Encodes a given string using Base32 encoding.
     *
     * @param str The string to be encoded.
     * @return The Base32 encoded string(Upper-Case).
     */
    static std::string Base32Encode(const std::string &str);
    /**
     * Encodes a given string using Base64 encoding.
     *
     * @param str The string to be encoded.
     * @return The Base64 encoded string.
     */
    static std::string Base64Encode(const std::string &str);
    /**
     * Decodes a Base16-encoded string.
     *
     * @param str The Base16-encoded string to decode(Upper/Lower-Case).
     * @return The decoded string.
     */
    static std::string Base16Decode(const std::string &str);
    /**
     * Decodes a Base32 encoded string.
     *
     * @param str The Base32 encoded string to decode(Upper-Case).
     * @return The decoded string.
     */
    static std::string Base32Decode(const std::string &str);
    /**
     * Decodes a Base64 encoded string.
     *
     * @param str The Base64 encoded string to decode.
     * @return The decoded string.
     */
    static std::string Base64Decode(const std::string &str);
};

} // namespace Scorpion