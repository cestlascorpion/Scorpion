#pragma once

#include <cstdio>
#include <string>
#include <tuple>

namespace Scorpion {

class SMX {
public:
    /**
     * Generates a SM2 key pair.
     *
     * @param pass The password for the private key file.
     * @param pub The file pointer to store the generated public key.
     * @param pem The file pointer to store the generated private key.
     * @return Returns 0 if success, or a non-zero value otherwise.
     */
    static int SM2KeyGen(const std::string &pass, FILE *pub, FILE *pem);
    /**
     * @brief Performs SM2 signing using a private key file.
     *
     * @param str The string to be signed.
     * @param pem The pointer to the private key file.
     * @param pass The password for the private key file.
     * @param id The optional ID string.
     * @return The signed string.
     */
    static std::string SM2Sign(const std::string &str, FILE *pem, const std::string &pass, const std::string &id = {});
    /**
     * Verifies a SM2 signature using a public key file.
     *
     * @param str The string to verify.
     * @param signature The signature to verify.
     * @param pub The pointer to the public key file.
     * @param id The optional ID string.
     * @return Returns 0 if the signature is valid, or a non-zero value otherwise.
     */
    static int SM2Verify(const std::string &str, const std::string &signature, FILE *pub, const std::string &id = {});
    /**
     * Encrypts the given string using SM2 encryption algorithm.
     *
     * @param str The string to be encrypted.
     * @param pub The pointer to the public key file.
     * @return The encrypted string.
     */
    static std::string SM2Encrypt(const std::string &str, FILE *pub);
    /**
     * Decrypts the given string using SM2 decryption algorithm.
     *
     * @param str The string to be decrypted.
     * @param pem The pointer to the private key file.
     * @param pass The password for the private key file.
     * @return The decrypted string.
     */
    static std::string SM2Decrypt(const std::string &str, FILE *pem, const std::string &pass);

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
     * @return The SM3 hash value as a string.
     */
    static std::string SM3HashFile(FILE *file);
    /**
     * Calculates the SM3 HMAC for the given string using the provided key.
     *
     * @param str The input string to calculate the HMAC hash for.
     * @param key The key used for HMAC calculation.
     * @return The SM3 HMAC value as a string.
     */
    static std::string SM3HMAC(const std::string &str, const std::string &key);
    /**
     * Calculates the SM3 HMAC of a file using the provided key.
     *
     * @param file The file to calculate the HMAC hash for.
     * @param key The key used for HMAC calculation.
     * @return The SM3 HMAC value as a string.
     */
    static std::string SM3HMACFile(FILE *file, const std::string &key);

public:
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
     * Decrypts a string using the SM4-CBC and SM3-HMAC algorithms.
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

public:
    /**
     * Generates a SM9 master key pair for signing (KGC).
     *
     * @param pass The password for the master private key file.
     * @param pub The file pointer to store the generated master public key.
     * @param pem The file pointer to store the generated master private key.
     * @return Returns 0 if success, or a non-zero value otherwise.
     */
    static int SM9SignMasterKeyGen(const std::string &pass, FILE *pub, FILE *pem);
    /**
     * Generates a SM9 master key pair for encrypting (KGC).
     *
     * @param pass The password for the master private key file.
     * @param pub The file pointer to write the generated master public key.
     * @param pem The file pointer to write the generated master private key.
     * @return Returns 0 if success, or a non-zero value otherwise.
     */
    static int SM9EncryptMasterKeyGen(const std::string &pass, FILE *pub, FILE *pem);
    /**
     * Generates an SM9 user key for signing.
     *
     * @param masterPass The password for the master private key file.
     * @param masterPem The master PEM file containing the master private key.
     * @param userPass The password for the user private key file.
     * @param userPem The user PEM file to be generated.
     * @param id The ID string.
     * @return Returns 0 if success, or a non-zero value otherwise.
     */
    static int SM9SignUserKeyGen(const std::string &masterPass, FILE *masterPem, const std::string &userPass,
                                 FILE *userPem, const std::string &id);
    /**
     * Generates an SM9 user pair for encrypting.
     *
     * @param masterPass The password for the master private key file.
     * @param masterPem The master PEM file containing the master private key.
     * @param userPass The password for the user private key file.
     * @param userPem The user PEM file to be generated.
     * @param id The ID string.
     * @return Returns 0 if success, or a non-zero value otherwise.
     */
    static int SM9EncryptUserKeyGen(const std::string &masterPass, FILE *masterPem, const std::string &userPass,
                                    FILE *userPem, const std::string &id);
    /**
     * @brief Performs SM9 signing using a signing private key file.
     *
     * @param str The string to be signed.
     * @param pem The pointer to the signing private key file.
     * @param pass The password for the signing private key file.
     * @return The signed string.
     */
    static std::string SM9Sign(const std::string &str, FILE *pem, const std::string &pass);
    /**
     * Verifies a SM9 signature using a signing public key file.
     *
     * @param str The string to verify.
     * @param signature The signature to verify.
     * @param pub The pointer to the signing public key file.
     * @param id The ID string.
     * @return Returns 0 if the signature is valid, or a non-zero value otherwise.
     */
    static int SM9Verify(const std::string &str, const std::string &signature, FILE *pub, const std::string &id);
    /**
     * Encrypts a given string using the SM9 encryption algorithm.
     *
     * @param str The string to be encrypted.
     * @param pub The public key file pointer.
     * @param id The ID string.
     * @return The encrypted string.
     */
    static std::string SM9Encrypt(const std::string &str, FILE *pub, const std::string &id);
    /**
     * Decrypts a string using the SM9 encryption algorithm.
     *
     * @param str The string to be decrypted.
     * @param userPass The user's password.
     * @param userPem The user's PEM file.
     * @param id The ID string.
     * @return The decrypted string.
     */
    static std::string SM9Decrypt(const std::string &str, const std::string &userPass, FILE *userPem,
                                  const std::string &id);
};

} // namespace Scorpion