#include "SMX.h"

#include <iostream>

#include "gmssl/aead.h"
#include "gmssl/asn1.h"
#include "gmssl/sm2.h"
#include "gmssl/sm3.h"
#include "gmssl/sm4.h"
#include "gmssl/sm9.h"

using namespace std;
using namespace Scorpion;

namespace detail {

struct BufferGuard {
    explicit BufferGuard(size_t size) {
        _buf = (uint8_t *)malloc(size);
    }
    ~BufferGuard() {
        if (_buf != nullptr) {
            free(_buf);
        }
    }
    uint8_t *_buf;
};

} // namespace detail

int SMX::SM2KeyGen(const string &pass, FILE *pub, FILE *pem) {
    if (pass.empty() || pub == nullptr || pem == nullptr) {
        return -1;
    }
    SM2_KEY key;
    auto ret = sm2_key_generate(&key);
    // assert(ret == 1 && "sm2_key_generate error");
    if (ret != 1) {
        return -1;
    }
    ret = sm2_private_key_info_encrypt_to_pem(&key, pass.c_str(), pem);
    // assert(ret == 1 && "sm2_private_key_info_encrypt_to_pem error");
    if (ret != 1) {
        return -1;
    }
    ret = sm2_public_key_info_to_pem(&key, pub);
    // assert(ret == 1 && "sm2_public_key_info_to_pem error");
    if (ret != 1) {
        return -1;
    }
    return 0;
}

string SMX::SM2Sign(const string &str, FILE *pem, const string &pass, const string &id) {
    if (str.empty() || pem == nullptr || pass.empty()) {
        return {};
    }
    SM2_KEY key;
    auto ret = sm2_private_key_info_decrypt_from_pem(&key, pass.c_str(), pem);
    // assert(ret == 1 && "sm2_private_key_info_decrypt_from_pem error");
    if (ret != 1) {
        return {};
    }
    SM2_SIGN_CTX sign_ctx;
    if (id.empty()) {
        ret = sm2_sign_init(&sign_ctx, &key, SM2_DEFAULT_ID, strlen(SM2_DEFAULT_ID));
        // assert(ret == 1 && "sm2_sign_init error");
        if (ret != 1) {
            return {};
        }
    } else {
        ret = sm2_sign_init(&sign_ctx, &key, id.c_str(), id.size());
        // assert(ret == 1 && "sm2_sign_init error");
        if (ret != 1) {
            return {};
        }
    }
    ret = sm2_sign_update(&sign_ctx, (const uint8_t *)str.c_str(), str.size());
    // assert(ret == 1 && "sm2_sign_update error");
    if (ret != 1) {
        return {};
    }
    uint8_t sig[SM2_MAX_SIGNATURE_SIZE];
    size_t siglen;
    ret = sm2_sign_finish(&sign_ctx, sig, &siglen);
    // assert(ret == 1 && "sm2_sign_finish error");
    if (ret != 1) {
        return {};
    }
    return {(char *)sig, siglen};
}

int SMX::SM2Verify(const string &str, const string &signature, FILE *pub, const string &id) {
    if (signature.empty() || pub == nullptr) {
        return -1;
    }
    SM2_KEY key;
    auto ret = sm2_public_key_info_from_pem(&key, pub);
    // assert(ret == 1 && "sm2_public_key_info_from_pem error");
    if (ret != 1) {
        return -1;
    }
    SM2_SIGN_CTX verify_ctx;
    if (id.empty()) {
        ret = sm2_verify_init(&verify_ctx, &key, SM2_DEFAULT_ID, strlen(SM2_DEFAULT_ID));
        // assert(ret == 1 && "sm2_verify_init error");
        if (ret != 1) {
            return -1;
        }
    } else {
        ret = sm2_verify_init(&verify_ctx, &key, id.c_str(), id.size());
        // assert(ret == 1 && "sm2_verify_init error");
        if (ret != 1) {
            return -1;
        }
    }
    ret = sm2_verify_update(&verify_ctx, (const uint8_t *)str.c_str(), str.size());
    // assert(ret == 1 && "sm2_verify_update error");
    if (ret != 1) {
        return -1;
    }
    ret = sm2_verify_finish(&verify_ctx, (const uint8_t *)signature.c_str(), signature.size());
    // assert(ret == 1 && "sm2_verify_finish error");
    if (ret != 1) {
        return -1;
    }
    return 0;
}

string SMX::SM2Encrypt(const string &str, FILE *pub) {
    if (str.empty() || str.size() > SM2_MAX_PLAINTEXT_SIZE || pub == nullptr) {
        return {};
    }
    SM2_KEY key;
    auto ret = sm2_public_key_info_from_pem(&key, pub);
    // assert(ret == 1 && "sm2_public_key_info_from_pem error");
    if (ret != 1) {
        return {};
    }
    uint8_t cipher[SM2_MAX_CIPHERTEXT_SIZE];
    size_t length = 0;
    ret = sm2_encrypt(&key, (const uint8_t *)str.c_str(), str.size(), cipher, &length);
    // assert(ret == 1 && "sm2_encrypt error");
    if (ret != 1) {
        return {};
    }
    return string{(char *)cipher, length};
}

string SMX::SM2Decrypt(const string &str, FILE *pem, const string &pass) {
    if (str.empty() || str.size() > SM2_MAX_CIPHERTEXT_SIZE || pem == nullptr || pass.empty()) {
        return {};
    }
    SM2_KEY key;
    auto ret = sm2_private_key_info_decrypt_from_pem(&key, pass.c_str(), pem);
    // assert(ret == 1 && "sm2_private_key_info_decrypt_from_pem error");
    if (ret != 1) {
        return {};
    }
    uint8_t plain[SM2_MAX_PLAINTEXT_SIZE];
    size_t length = 0;
    ret = sm2_decrypt(&key, (const uint8_t *)str.c_str(), str.size(), plain, &length);
    // assert(ret == 1 && "sm2_decrypt error");
    if (ret != 1) {
        return {};
    }
    return string{(char *)plain, length};
}

string SMX::SM3Hash(const string &str) {
    if (str.empty()) {
        return {};
    }
    char digest[SM3_DIGEST_SIZE];
    sm3_digest((const uint8_t *)str.c_str(), str.size(), (uint8_t *)digest);
    return {digest, SM3_DIGEST_SIZE};
}

string SMX::SM3HashFile(FILE *file) {
    if (file == nullptr) {
        return {};
    }
    char digest[SM3_DIGEST_SIZE];
    uint8_t buf[4096];
    size_t len;
    SM3_CTX sm3_ctx;
    sm3_init(&sm3_ctx);
    while ((len = fread((void *)buf, 1, sizeof(buf), file)) > 0) {
        sm3_update(&sm3_ctx, buf, len);
    }
    sm3_finish(&sm3_ctx, (uint8_t *)digest);
    return {digest, SM3_DIGEST_SIZE};
}

string SMX::SM3HMAC(const string &str, const string &key) {
    if (str.empty() || key.empty() || key.size() > SM3_DIGEST_SIZE) {
        return {};
    }
    char mac[SM3_HMAC_SIZE];
    SM3_HMAC_CTX ctx;
    sm3_hmac_init(&ctx, (const uint8_t *)key.c_str(), key.size());
    sm3_hmac_update(&ctx, (const uint8_t *)str.c_str(), str.size());
    sm3_hmac_finish(&ctx, (uint8_t *)mac);
    return {mac, SM3_HMAC_SIZE};
}

string SMX::SM3HMACFile(FILE *file, const string &key) {
    if (file == nullptr || key.empty() || key.size() > SM3_DIGEST_SIZE) {
        return {};
    }
    char digest[SM3_HMAC_SIZE];
    uint8_t buf[4096];
    size_t len;
    SM3_HMAC_CTX ctx;
    sm3_hmac_init(&ctx, (const uint8_t *)key.c_str(), key.size());
    while ((len = fread((void *)buf, 1, sizeof(buf), file)) > 0) {
        sm3_hmac_update(&ctx, buf, len);
    }
    sm3_hmac_finish(&ctx, (uint8_t *)digest);
    return {digest, SM3_DIGEST_SIZE};
}

string SMX::SM4CBCEncrypt(const string &str, const string &key, const string &iv) {
    if (str.empty() || key.size() != SM4_KEY_SIZE || iv.size() != SM4_BLOCK_SIZE) {
        return {};
    }
    detail::BufferGuard guard(str.size() + SM4_BLOCK_SIZE);
    SM4_CBC_CTX ctx;
    auto ret = sm4_cbc_encrypt_init(&ctx, (const uint8_t *)key.c_str(), (const uint8_t *)iv.c_str());
    // assert(ret == 1 && "sm4_cbc_encrypt_init error");
    if (ret != 1) {
        return {};
    }
    size_t length = 0;
    ret = sm4_cbc_encrypt_update(&ctx, (const uint8_t *)str.c_str(), str.size(), guard._buf, &length);
    // assert(ret == 1 && "sm4_cbc_encrypt_update error");
    if (ret != 1) {
        return {};
    }
    size_t tail = 0;
    ret = sm4_cbc_encrypt_finish(&ctx, guard._buf + length, &tail);
    // assert(ret == 1 && "sm4_cbc_encrypt_finish error");
    if (ret != 1) {
        return {};
    }
    return string{(char *)guard._buf, length + tail};
}

string SMX::SM4CTREncrypt(const string &str, const string &key, const string &iv) {
    if (str.empty() || key.size() != SM4_KEY_SIZE || iv.size() != SM4_BLOCK_SIZE) {
        return {};
    }
    detail::BufferGuard guard(str.size() + SM4_BLOCK_SIZE);
    SM4_CTR_CTX ctx;
    auto ret = sm4_ctr_encrypt_init(&ctx, (const uint8_t *)key.c_str(), (const uint8_t *)iv.c_str());
    // assert(ret == 1 && "sm4_ctr_encrypt_init error");
    if (ret != 1) {
        return {};
    }
    size_t length = 0;
    ret = sm4_ctr_encrypt_update(&ctx, (const uint8_t *)str.c_str(), str.size(), guard._buf, &length);
    // assert(ret == 1 && "sm4_ctr_encrypt_update error");
    if (ret != 1) {
        return {};
    }
    size_t tail = 0;
    ret = sm4_ctr_encrypt_finish(&ctx, guard._buf + length, &tail);
    // assert(ret == 1 && "sm4_ctr_encrypt_finish error");
    if (ret != 1) {
        return {};
    }
    return string{(char *)guard._buf, length + tail};
}

string SMX::SM4CBCDecrypt(const string &str, const string &key, const string &iv) {
    if (str.empty() || key.size() != SM4_KEY_SIZE || iv.size() != SM4_BLOCK_SIZE) {
        return {};
    }
    detail::BufferGuard guard(str.size());
    SM4_CBC_CTX ctx;
    auto ret = sm4_cbc_decrypt_init(&ctx, (const uint8_t *)key.c_str(), (const uint8_t *)iv.c_str());
    // assert(ret == 1 && "sm4_cbc_decrypt_init error");
    if (ret != 1) {
        return {};
    }
    size_t length = 0;
    ret = sm4_cbc_decrypt_update(&ctx, (const uint8_t *)str.c_str(), str.size(), guard._buf, &length);
    // assert(ret == 1 && "sm4_cbc_decrypt_update error");
    if (ret != 1) {
        return {};
    }
    size_t tail = 0;
    ret = sm4_cbc_decrypt_finish(&ctx, guard._buf + length, &tail);
    // assert(ret == 1 && "sm4_cbc_decrypt_finish error");
    if (ret != 1) {
        return {};
    }
    return string{(char *)guard._buf, length + tail};
}

string SMX::SM4CTRDecrypt(const string &str, const string &key, const string &iv) {
    if (str.empty() || key.size() != SM4_KEY_SIZE || iv.size() != SM4_BLOCK_SIZE) {
        return {};
    }
    detail::BufferGuard guard(str.size());
    SM4_CTR_CTX ctx;
    auto ret = sm4_ctr_decrypt_init(&ctx, (const uint8_t *)key.c_str(), (const uint8_t *)iv.c_str());
    // assert(ret == 1 && "sm4_ctr_decrypt_init error");
    if (ret != 1) {
        return {};
    }
    size_t length = 0;
    ret = sm4_ctr_decrypt_update(&ctx, (const uint8_t *)str.c_str(), str.size(), guard._buf, &length);
    // assert(ret == 1 && "sm4_ctr_decrypt_update error");
    if (ret != 1) {
        return {};
    }
    size_t tail = 0;
    ret = sm4_ctr_decrypt_finish(&ctx, guard._buf + length, &tail);
    // assert(ret == 1 && "sm4_ctr_decrypt_finish error");
    if (ret != 1) {
        return {};
    }
    return string{(char *)guard._buf, length + tail};
}

string SMX::SM4GCMEncrypt(const string &str, const string &key, const string &iv, const string &aad) {
    if (str.empty() || key.size() != SM4_KEY_SIZE || iv.size() < SM4_GCM_MIN_IV_SIZE ||
        iv.size() > SM4_GCM_MAX_IV_SIZE) {
        return {};
    }
    detail::BufferGuard guard(str.size() + GHASH_SIZE);
    SM4_GCM_CTX ctx;
    auto ret = sm4_gcm_encrypt_init(&ctx, (const uint8_t *)key.c_str(), key.size(), (const uint8_t *)iv.c_str(),
                                    iv.size(), (const uint8_t *)aad.c_str(), aad.size(), GHASH_SIZE);
    // assert(ret == 1 && "sm4_gcm_encrypt_init error");
    if (ret != 1) {
        return {};
    }
    size_t length = 0;
    ret = sm4_gcm_encrypt_update(&ctx, (const uint8_t *)str.c_str(), str.size(), guard._buf, &length);
    // assert(ret == 1 && "sm4_gcm_encrypt_update error");
    if (ret != 1) {
        return {};
    }
    size_t tail = 0;
    ret = sm4_gcm_encrypt_finish(&ctx, guard._buf + length, &tail);
    // assert(ret == 1 && "sm4_gcm_encrypt_finish error");
    if (ret != 1) {
        return {};
    }
    return string{(char *)guard._buf, length + tail};
}

string SMX::SM4CBCAndSM3HMACEncrypt(const string &str, const string &key, const string &iv, const string &aad) {
    if (str.empty() || key.size() != SM4_KEY_SIZE + SM3_HMAC_SIZE || iv.size() != SM4_BLOCK_SIZE) {
        return {};
    }
    detail::BufferGuard guard(str.size() + SM4_KEY_SIZE + SM3_HMAC_SIZE);
    SM4_CBC_SM3_HMAC_CTX ctx;
    auto ret =
        sm4_cbc_sm3_hmac_encrypt_init(&ctx, (const uint8_t *)key.c_str(), key.size(), (const uint8_t *)iv.c_str(),
                                      iv.size(), (const uint8_t *)aad.c_str(), aad.size());
    // assert(ret == 1 && "sm4_cbc_sm3_hmac_encrypt_init error");
    if (ret != 1) {
        return {};
    }
    size_t length = 0;
    ret = sm4_cbc_sm3_hmac_encrypt_update(&ctx, (const uint8_t *)str.c_str(), str.size(), guard._buf, &length);
    // assert(ret == 1 && "sm4_cbc_sm3_hmac_encrypt_update error");
    if (ret != 1) {
        return {};
    }
    size_t tail = 0;
    ret = sm4_cbc_sm3_hmac_encrypt_finish(&ctx, guard._buf + length, &tail);
    // assert(ret == 1 && "sm4_cbc_sm3_hmac_encrypt_finish error");
    if (ret != 1) {
        return {};
    }
    return string{(char *)guard._buf, length + tail};
}

string SMX::SM4CTRAndSM3HMACEncrypt(const string &str, const string &key, const string &iv, const string &aad) {
    if (str.empty() || key.size() != SM4_KEY_SIZE + SM3_HMAC_SIZE || iv.size() != SM4_BLOCK_SIZE) {
        return {};
    }
    detail::BufferGuard guard(str.size() + SM4_KEY_SIZE + SM3_HMAC_SIZE);
    SM4_CTR_SM3_HMAC_CTX ctx;
    auto ret =
        sm4_ctr_sm3_hmac_encrypt_init(&ctx, (const uint8_t *)key.c_str(), key.size(), (const uint8_t *)iv.c_str(),
                                      iv.size(), (const uint8_t *)aad.c_str(), aad.size());
    // assert(ret == 1 && "sm4_ctr_sm3_hmac_encrypt_init error");
    if (ret != 1) {
        return {};
    }
    size_t length = 0;
    ret = sm4_ctr_sm3_hmac_encrypt_update(&ctx, (const uint8_t *)str.c_str(), str.size(), guard._buf, &length);
    // assert(ret == 1 && "sm4_ctr_sm3_hmac_encrypt_update error");
    if (ret != 1) {
        return {};
    }
    size_t tail = 0;
    ret = sm4_ctr_sm3_hmac_encrypt_finish(&ctx, guard._buf + length, &tail);
    // assert(ret == 1 && "sm4_ctr_sm3_hmac_encrypt_finish error");
    if (ret != 1) {
        return {};
    }
    return string{(char *)guard._buf, length + tail};
}

string SMX::SM4GCMDecrypt(const string &str, const string &key, const string &iv, const string &aad) {
    if (str.empty() || key.size() != SM4_KEY_SIZE || iv.size() < SM4_GCM_MIN_IV_SIZE ||
        iv.size() > SM4_GCM_MAX_IV_SIZE) {
        return {};
    }
    detail::BufferGuard guard(str.size());
    SM4_GCM_CTX ctx;
    auto ret = sm4_gcm_decrypt_init(&ctx, (const uint8_t *)key.c_str(), key.size(), (const uint8_t *)iv.c_str(),
                                    iv.size(), (const uint8_t *)aad.c_str(), aad.size(), GHASH_SIZE);
    // assert(ret == 1 && "sm4_gcm_decrypt_init error");
    if (ret != 1) {
        return {};
    }
    size_t length = 0;
    ret = sm4_gcm_decrypt_update(&ctx, (const uint8_t *)str.c_str(), str.size(), guard._buf, &length);
    // assert(ret == 1 && "sm4_gcm_decrypt_update error");
    if (ret != 1) {
        return {};
    }
    size_t tail = 0;
    ret = sm4_gcm_decrypt_finish(&ctx, guard._buf + length, &tail);
    // assert(ret == 1 && "sm4_gcm_decrypt_finish error");
    if (ret != 1) {
        return {};
    }
    return string{(char *)guard._buf, length + tail};
}

string SMX::SM4CBCAndSM3HMACDecrypt(const string &str, const string &key, const string &iv, const string &aad) {
    if (str.empty() || key.size() != SM4_KEY_SIZE + SM3_HMAC_SIZE || iv.size() != SM4_BLOCK_SIZE) {
        return {};
    }
    detail::BufferGuard guard(str.size());
    SM4_CBC_SM3_HMAC_CTX ctx;
    auto ret =
        sm4_cbc_sm3_hmac_decrypt_init(&ctx, (const uint8_t *)key.c_str(), key.size(), (const uint8_t *)iv.c_str(),
                                      iv.size(), (const uint8_t *)aad.c_str(), aad.size());
    // assert(ret == 1 && "sm4_cbc_sm3_hmac_decrypt_init error");
    if (ret != 1) {
        return {};
    }
    size_t length = 0;
    ret = sm4_cbc_sm3_hmac_decrypt_update(&ctx, (const uint8_t *)str.c_str(), str.size(), guard._buf, &length);
    // assert(ret == 1 && "sm4_cbc_sm3_hmac_decrypt_update error");
    if (ret != 1) {
        return {};
    }
    size_t tail = 0;
    ret = sm4_cbc_sm3_hmac_decrypt_finish(&ctx, guard._buf + length, &tail);
    // assert(ret == 1 && "sm4_cbc_sm3_hmac_decrypt_finish error");
    if (ret != 1) {
        return {};
    }
    return string{(char *)guard._buf, length + tail};
}

string SMX::SM4CTRAndSM3HMACDecrypt(const string &str, const string &key, const string &iv, const string &aad) {
    if (str.empty() || key.size() != SM4_KEY_SIZE + SM3_HMAC_SIZE || iv.size() != SM4_BLOCK_SIZE) {
        return {};
    }
    detail::BufferGuard guard(str.size());
    SM4_CTR_SM3_HMAC_CTX ctx;
    auto ret =
        sm4_ctr_sm3_hmac_decrypt_init(&ctx, (const uint8_t *)key.c_str(), key.size(), (const uint8_t *)iv.c_str(),
                                      iv.size(), (const uint8_t *)aad.c_str(), aad.size());
    // assert(ret == 1 && "sm4_ctr_sm3_hmac_decrypt_init error");
    if (ret != 1) {
        return {};
    }
    size_t length = 0;
    ret = sm4_ctr_sm3_hmac_decrypt_update(&ctx, (const uint8_t *)str.c_str(), str.size(), guard._buf, &length);
    // assert(ret == 1 && "sm4_ctr_sm3_hmac_decrypt_update error");
    if (ret != 1) {
        return {};
    }
    size_t tail = 0;
    ret = sm4_ctr_sm3_hmac_decrypt_finish(&ctx, guard._buf + length, &tail);
    // assert(ret == 1 && "sm4_ctr_sm3_hmac_decrypt_finish error");
    if (ret != 1) {
        return {};
    }
    return string{(char *)guard._buf, length + tail};
}

int SMX::SM9SignMasterKeyGen(const string &pass, FILE *pub, FILE *pem) {
    if (pass.empty() || pub == nullptr || pem == nullptr) {
        return -1;
    }
    SM9_SIGN_MASTER_KEY sign_msk;
    auto ret = sm9_sign_master_key_generate(&sign_msk);
    // assert(ret == 1 && "sm9_sign_master_key_generate error");
    if (ret != 1) {
        return -1;
    }
    ret = sm9_sign_master_key_info_encrypt_to_pem(&sign_msk, pass.c_str(), pem);
    // assert(ret == 1 && "sm9_sign_master_key_info_encrypt_to_pem error");
    if (ret != 1) {
        return -1;
    }
    ret = sm9_sign_master_public_key_to_pem(&sign_msk, pub);
    // assert(ret == 1 && "sm9_sign_master_public_key_to_pem error");
    if (ret != 1) {
        return -1;
    }
    return 0;
}

int SMX::SM9EncryptMasterKeyGen(const string &pass, FILE *pub, FILE *pem) {
    if (pass.empty() || pub == nullptr || pem == nullptr) {
        return -1;
    }
    SM9_ENC_MASTER_KEY enc_msk;
    auto ret = sm9_enc_master_key_generate(&enc_msk);
    // assert(ret == 1 && "sm9_enc_master_key_generate error");
    if (ret != 1) {
        return -1;
    }
    ret = sm9_enc_master_key_info_encrypt_to_pem(&enc_msk, pass.c_str(), pem);
    // assert(ret == 1 && "sm9_enc_master_key_info_encrypt_to_pem error");
    if (ret != 1) {
        return -1;
    }
    ret = sm9_enc_master_public_key_to_pem(&enc_msk, pub);
    // assert(ret == 1 && "sm9_enc_master_public_key_to_pem error");
    if (ret != 1) {
        return -1;
    }
    return 0;
}

int SMX::SM9SignUserKeyGen(const string &masterPass, FILE *masterPem, const string &userPass, FILE *userPem,
                           const string &id) {
    if (masterPass.empty() || masterPem == nullptr || userPass.empty() || userPem == nullptr) {
        return {};
    }
    SM9_SIGN_MASTER_KEY sign_msk;
    auto ret = sm9_sign_master_key_info_decrypt_from_pem(&sign_msk, masterPass.c_str(), masterPem);
    // assert(ret == 1 && "sm9_sign_master_key_info_decrypt_from_pem error");
    if (ret != 1) {
        return {};
    }
    SM9_SIGN_KEY sign_key;
    ret = sm9_sign_master_key_extract_key(&sign_msk, id.c_str(), id.size(), &sign_key);
    // assert(ret == 1 && "sm9_sign_master_key_extract_key error");
    if (ret != 1) {
        return {};
    }
    ret = sm9_sign_key_info_encrypt_to_pem(&sign_key, userPass.c_str(), userPem);
    // assert(ret == 1 && "sm9_sign_key_info_encrypt_to_pem error");
    if (ret != 1) {
        return {};
    }
    return 0;
}

int SMX::SM9EncryptUserKeyGen(const string &masterPass, FILE *masterPem, const string &userPass, FILE *userPem,
                              const string &id) {
    if (masterPass.empty() || masterPem == nullptr || userPass.empty() || userPem == nullptr) {
        return {};
    }
    SM9_ENC_MASTER_KEY enc_msk;
    auto ret = sm9_enc_master_key_info_decrypt_from_pem(&enc_msk, masterPass.c_str(), masterPem);
    // assert(ret == 1 && "sm9_enc_master_key_info_decrypt_from_pem error");
    if (ret != 1) {
        return {};
    }
    SM9_ENC_KEY enc_key;
    ret = sm9_enc_master_key_extract_key(&enc_msk, id.c_str(), id.size(), &enc_key);
    // assert(ret == 1 && "sm9_enc_master_key_extract_key error");
    if (ret != 1) {
        return {};
    }
    ret = sm9_enc_key_info_encrypt_to_pem(&enc_key, userPass.c_str(), userPem);
    // assert(ret == 1 && "sm9_enc_key_info_encrypt_to_pem error");
    if (ret != 1) {
        return {};
    }
    return 0;
}

string SMX::SM9Sign(const string &str, FILE *pem, const string &pass) {
    if (str.empty() || pem == nullptr || pass.empty()) {
        return {};
    }
    SM9_SIGN_KEY key;
    auto ret = sm9_sign_key_info_decrypt_from_pem(&key, pass.c_str(), pem);
    // assert(ret == 1 && "sm9_sign_key_info_decrypt_from_pem error");
    if (ret != 1) {
        return {};
    }
    SM9_SIGN_CTX ctx;
    ret = sm9_sign_init(&ctx);
    // assert(ret == 1 && "sm9_sign_init error");
    if (ret != 1) {
        return {};
    }
    ret = sm9_sign_update(&ctx, (const uint8_t *)str.c_str(), str.size());
    // assert(ret == 1 && "sm9_sign_update error");
    if (ret != 1) {
        return {};
    }
    uint8_t sig[SM9_SIGNATURE_SIZE];
    size_t siglen;
    ret = sm9_sign_finish(&ctx, &key, sig, &siglen);
    // assert(ret == 1 && "sm9_sign_finish error");
    if (ret != 1) {
        return {};
    }
    return {(char *)sig, siglen};
}

int SMX::SM9Verify(const string &str, const string &signature, FILE *pub, const string &id) {
    if (str.empty() || signature.empty() || pub == nullptr || id.empty()) {
        return -1;
    }
    SM9_SIGN_MASTER_KEY mpk;
    auto ret = sm9_sign_master_public_key_from_pem(&mpk, pub);
    // assert(ret == 1 && "sm9_sign_master_public_key_from_pem error");
    if (ret != 1) {
        return -1;
    }
    SM9_SIGN_CTX ctx;
    ret = sm9_verify_init(&ctx);
    // assert(ret == 1 && "sm9_verify_init error");
    if (ret != 1) {
        return -1;
    }
    ret = sm9_verify_update(&ctx, (const uint8_t *)str.c_str(), str.size());
    // assert(ret == 1 && "sm9_verify_update error");
    if (ret != 1) {
        return -1;
    }
    ret = sm9_verify_finish(&ctx, (const uint8_t *)signature.c_str(), signature.size(), &mpk, id.c_str(), id.size());
    // assert(ret == 1 && "sm9_verify_finish error");
    if (ret != 1) {
        return -1;
    }
    return 0;
}

string SMX::SM9Encrypt(const string &str, FILE *pub, const string &id) {
    if (str.empty() || str.size() > SM9_MAX_PLAINTEXT_SIZE || pub == nullptr || id.empty()) {
        return {};
    }
    SM9_ENC_MASTER_KEY mpk;
    auto ret = sm9_enc_master_public_key_from_pem(&mpk, pub);
    // assert(ret == 1 && "sm9_enc_master_public_key_from_pem error");
    if (ret != 1) {
        return {};
    }
    uint8_t cipher[SM9_MAX_CIPHERTEXT_SIZE];
    size_t cipherlen;
    ret = sm9_encrypt(&mpk, id.c_str(), id.size(), (const uint8_t *)str.c_str(), str.size(), cipher, &cipherlen);
    // assert(ret == 1 && "sm9_encrypt error");
    if (ret != 1) {
        return {};
    }
    return {(char *)cipher, cipherlen};
}

string SMX::SM9Decrypt(const string &str, const string &userPass, FILE *userPem, const string &id) {
    if (str.empty() || str.size() > SM9_MAX_CIPHERTEXT_SIZE || userPass.empty() || userPem == nullptr || id.empty()) {
        return {};
    }
    SM9_ENC_KEY key;
    auto ret = sm9_enc_key_info_decrypt_from_pem(&key, userPass.c_str(), userPem);
    // assert(ret == 1 && "sm9_enc_key_info_decrypt_from_pem error");
    if (ret != 1) {
        return {};
    }
    uint8_t plain[SM9_MAX_PLAINTEXT_SIZE];
    size_t plainlen;
    ret = sm9_decrypt(&key, id.c_str(), id.size(), (const uint8_t *)str.c_str(), str.size(), plain, &plainlen);
    // assert(ret == 1 && "sm9_decrypt error");
    if (ret != 1) {
        return {};
    }
    return {(char *)plain, plainlen};
}