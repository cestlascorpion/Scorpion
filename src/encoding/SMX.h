#pragma once

#include <cstdio>
#include <string>
#include <tuple>

namespace Scorpion {

class SMX {
public:
    /**
     * Calculates the SM3 hash value for the given string.
     *
     * @param str The input string to be hashed.
     * @return The SM3 hash value as a string.
     */
    static std::string SM3Hash(const std::string &str);
    /**
     * Calculates the SM3 hash value of a file.
     *
     * @param file A pointer to the file to be hashed.
     * @return The SM3 hash value of the file as a string.
     */
    static std::string SM3HashFile(FILE *file);
    /**
     * Calculates the SM3 HMAC (Hash-based Message Authentication Code) for the given string and key.
     *
     * @param str The input string to be hashed.
     * @param key The key used for HMAC calculation.
     * @return The SM3 HMAC value as a string.
     */
    static std::string SM3HMAC(const std::string &str, const std::string &key);
    /**
     * Calculates the SM3 HMAC hash of a file using the provided key.
     *
     * @param file The file to calculate the HMAC hash for.
     * @param key The key to use for the HMAC calculation.
     * @return The SM3 HMAC hash of the file as a string.
     */
    static std::string SM3HMACFile(FILE *file, const std::string &key);
    /**
     * Encrypts a string using the SM4-CBC encryption algorithm(padding mode: PKCS7).
     *
     * @param str The string to be encrypted.
     * @param key The encryption key.
     * @param iv The initialization vector.
     * @return The encrypted string.
     */
    static std::string SM4CBCEncrypt(const std::string &str, const std::string &key, const std::string &iv);
    /**
     * Encrypts a string using the SM4-CTR encryption algorithm(padding mode: None).
     *
     * @param str The string to be encrypted.
     * @param key The encryption key.
     * @param iv The initialization vector.
     * @return The encrypted string.
     */
    static std::string SM4CTREncrypt(const std::string &str, const std::string &key, const std::string &iv);
    /**
     * Encrypts a string using the SM4-GCM encryption algorithm(padding mode: None).
     *
     * @param str The string to be encrypted.
     * @param key The encryption key.
     * @param iv The initialization vector.
     * @param aad The additional authenticated data.
     * @return The encrypted string(cipher and mac).
     */
    static std::string SM4GCMEncrypt(const std::string &str, const std::string &key, const std::string &iv,
                                     const std::string &aad);
    /**
     * Decrypts a string using the SM4-CBC encryption algorithm(padding mode: PKCS7).
     *
     * @param str The string to be decrypted.
     * @param key The encryption key.
     * @param iv The initialization vector.
     * @return The decrypted string.
     */
    static std::string SM4CBCDecrypt(const std::string &str, const std::string &key, const std::string &iv);
    /**
     * Decrypts a string using the SM4-CTR mode encryption algorithm(padding mode: None).
     *
     * @param str The string to be decrypted.
     * @param key The encryption key.
     * @param iv The initialization vector.
     * @return The decrypted string.
     */
    static std::string SM4CTRDecrypt(const std::string &str, const std::string &key, const std::string &iv);
    /**
     * Decrypts a string using the SM4-GCM encryption algorithm(padding mode: None).
     *
     * @param str The string to be decrypted.
     * @param key The encryption key.
     * @param iv The initialization vector.
     * @param aad The additional authenticated data.
     * @return The decrypted string.
     */
    static std::string SM4GCMDecrypt(const std::string &str, const std::string &key, const std::string &iv,
                                     const std::string &aad);
    /**
     * Encrypts a string using SM4-CBC and SM3-HMAC algorithms.
     *
     * @param str The string to be encrypted.
     * @param key The encryption key.
     * @param iv The initialization vector.
     * @param aad The additional authenticated data.
     * @return The encrypted string(cipher and mac).
     */
    static std::string SM4CBCAndSM3HMACEncrypt(const std::string &str, const std::string &key, const std::string &iv,
                                               const std::string &aad);
    /**
     * Encrypts a string using SM4-CTR and SM3-HMAC algorithms.
     *
     * @param str The string to be encrypted.
     * @param key The encryption key.
     * @param iv The initialization vector.
     * @param aad The additional authenticated data.
     * @return The encrypted string(cipher and mac).
     */
    static std::string SM4CTRAndSM3HMACEncrypt(const std::string &str, const std::string &key, const std::string &iv,
                                               const std::string &aad);
    /**
     * Decrypts a string using the SM4-CBC and SM3-HMAC encryption algorithms.
     *
     * @param str The string to be decrypted.
     * @param key The encryption key.
     * @param iv The initialization vector.
     * @param aad The additional authenticated data.
     * @return The decrypted string.
     */
    static std::string SM4CBCAndSM3HMACDecrypt(const std::string &str, const std::string &key, const std::string &iv,
                                               const std::string &aad);
    /**
     * Decrypts a string using the SM4-CTR and SM3-HMAC algorithms.
     *
     * @param str The string to be decrypted.
     * @param key The encryption key.
     * @param iv The initialization vector.
     * @param aad The additional authenticated data.
     * @return The decrypted string.
     */
    static std::string SM4CTRAndSM3HMACDecrypt(const std::string &str, const std::string &key, const std::string &iv,
                                               const std::string &aad);
};

} // namespace Scorpion