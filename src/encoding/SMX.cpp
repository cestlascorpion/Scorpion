#include "SMX.h"

#include <cassert>

#include "gmssl/aead.h"
#include "gmssl/sm3.h"
#include "gmssl/sm4.h"

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
    assert(ret == 1 && "sm4_cbc_encrypt_init error");

    size_t length = 0;
    ret = sm4_cbc_encrypt_update(&ctx, (const uint8_t *)str.c_str(), str.size(), guard._buf, &length);
    assert(ret == 1 && "sm4_cbc_encrypt_update error");

    size_t tail = 0;
    ret = sm4_cbc_encrypt_finish(&ctx, guard._buf + length, &tail);
    assert(ret == 1 && "sm4_cbc_encrypt_finish error");

    return string{(char *)guard._buf, length + tail};
}

string SMX::SM4CTREncrypt(const string &str, const string &key, const string &iv) {
    if (str.empty() || key.size() != SM4_KEY_SIZE || iv.size() != SM4_BLOCK_SIZE) {
        return {};
    }

    detail::BufferGuard guard(str.size() + SM4_BLOCK_SIZE);

    SM4_CTR_CTX ctx;
    auto ret = sm4_ctr_encrypt_init(&ctx, (const uint8_t *)key.c_str(), (const uint8_t *)iv.c_str());
    assert(ret == 1 && "sm4_ctr_encrypt_init error");

    size_t length = 0;
    ret = sm4_ctr_encrypt_update(&ctx, (const uint8_t *)str.c_str(), str.size(), guard._buf, &length);
    assert(ret == 1 && "sm4_ctr_encrypt_update error");

    size_t tail = 0;
    ret = sm4_ctr_encrypt_finish(&ctx, guard._buf + length, &tail);
    assert(ret == 1 && "sm4_ctr_encrypt_finish error");

    return string{(char *)guard._buf, length + tail};
}

string SMX::SM4CBCDecrypt(const string &str, const string &key, const string &iv) {
    if (str.empty() || key.size() != SM4_KEY_SIZE || iv.size() != SM4_BLOCK_SIZE) {
        return {};
    }

    detail::BufferGuard guard(str.size());

    SM4_CBC_CTX ctx;
    auto ret = sm4_cbc_decrypt_init(&ctx, (const uint8_t *)key.c_str(), (const uint8_t *)iv.c_str());
    assert(ret == 1 && "sm4_cbc_decrypt_init error");

    size_t length = 0;
    ret = sm4_cbc_decrypt_update(&ctx, (const uint8_t *)str.c_str(), str.size(), guard._buf, &length);
    assert(ret == 1 && "sm4_cbc_decrypt_update error");

    size_t tail = 0;
    ret = sm4_cbc_decrypt_finish(&ctx, guard._buf + length, &tail);
    assert(ret == 1 && "sm4_cbc_decrypt_finish error");

    return string{(char *)guard._buf, length + tail};
}

string SMX::SM4CTRDecrypt(const string &str, const string &key, const string &iv) {
    if (str.empty() || key.size() != SM4_KEY_SIZE || iv.size() != SM4_BLOCK_SIZE) {
        return {};
    }

    detail::BufferGuard guard(str.size());

    SM4_CTR_CTX ctx;
    auto ret = sm4_ctr_decrypt_init(&ctx, (const uint8_t *)key.c_str(), (const uint8_t *)iv.c_str());
    assert(ret == 1 && "sm4_ctr_decrypt_init error");

    size_t length = 0;
    ret = sm4_ctr_decrypt_update(&ctx, (const uint8_t *)str.c_str(), str.size(), guard._buf, &length);
    assert(ret == 1 && "sm4_ctr_decrypt_update error");

    size_t tail = 0;
    ret = sm4_ctr_decrypt_finish(&ctx, guard._buf + length, &tail);
    assert(ret == 1 && "sm4_ctr_decrypt_finish error");

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
    assert(ret == 1 && "sm4_gcm_encrypt_init error");

    size_t length = 0;
    ret = sm4_gcm_encrypt_update(&ctx, (const uint8_t *)str.c_str(), str.size(), guard._buf, &length);
    assert(ret == 1 && "sm4_gcm_encrypt_update error");

    size_t tail = 0;
    ret = sm4_gcm_encrypt_finish(&ctx, guard._buf + length, &tail);
    assert(ret == 1 && "sm4_gcm_encrypt_finish error");

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
    assert(ret == 1 && "sm4_cbc_sm3_hmac_encrypt_init error");

    size_t length = 0;
    ret = sm4_cbc_sm3_hmac_encrypt_update(&ctx, (const uint8_t *)str.c_str(), str.size(), guard._buf, &length);
    assert(ret == 1 && "sm4_cbc_sm3_hmac_encrypt_update error");

    size_t tail = 0;
    ret = sm4_cbc_sm3_hmac_encrypt_finish(&ctx, guard._buf + length, &tail);
    assert(ret == 1 && "sm4_cbc_sm3_hmac_encrypt_finish error");

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
    assert(ret == 1 && "sm4_ctr_sm3_hmac_encrypt_init error");

    size_t length = 0;
    ret = sm4_ctr_sm3_hmac_encrypt_update(&ctx, (const uint8_t *)str.c_str(), str.size(), guard._buf, &length);
    assert(ret == 1 && "sm4_ctr_sm3_hmac_encrypt_update error");

    size_t tail = 0;
    ret = sm4_ctr_sm3_hmac_encrypt_finish(&ctx, guard._buf + length, &tail);
    assert(ret == 1 && "sm4_ctr_sm3_hmac_encrypt_finish error");

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
    assert(ret == 1 && "sm4_gcm_decrypt_init error");

    size_t length = 0;
    ret = sm4_gcm_decrypt_update(&ctx, (const uint8_t *)str.c_str(), str.size(), guard._buf, &length);
    assert(ret == 1 && "sm4_gcm_decrypt_update error");

    size_t tail = 0;
    ret = sm4_gcm_decrypt_finish(&ctx, guard._buf + length, &tail);
    assert(ret == 1 && "sm4_gcm_decrypt_finish error");

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
    assert(ret == 1 && "sm4_cbc_sm3_hmac_decrypt_init error");

    size_t length = 0;
    ret = sm4_cbc_sm3_hmac_decrypt_update(&ctx, (const uint8_t *)str.c_str(), str.size(), guard._buf, &length);
    assert(ret == 1 && "sm4_cbc_sm3_hmac_decrypt_update error");

    size_t tail = 0;
    ret = sm4_cbc_sm3_hmac_decrypt_finish(&ctx, guard._buf + length, &tail);
    assert(ret == 1 && "sm4_cbc_sm3_hmac_decrypt_finish error");

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
    assert(ret == 1 && "sm4_ctr_sm3_hmac_decrypt_init error");

    size_t length = 0;
    ret = sm4_ctr_sm3_hmac_decrypt_update(&ctx, (const uint8_t *)str.c_str(), str.size(), guard._buf, &length);
    assert(ret == 1 && "sm4_ctr_sm3_hmac_decrypt_update error");

    size_t tail = 0;
    ret = sm4_ctr_sm3_hmac_decrypt_finish(&ctx, guard._buf + length, &tail);
    assert(ret == 1 && "sm4_ctr_sm3_hmac_decrypt_finish error");

    return string{(char *)guard._buf, length + tail};
}
