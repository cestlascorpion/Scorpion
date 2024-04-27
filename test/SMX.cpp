#include "SMX.h"
#include "BaseX.h"
#include <cassert>
#include <iostream>

#define DEBUG 1

using namespace std;
using namespace Scorpion;

void TestSM3() {
    {
        const string str = "A";
        auto digest = SMX::SM3Hash(str);
        assert(BaseEncoding::Base16Encode(digest) ==
               "20882E95C4DD0CF1B2BC3E84E95E7C1465D71D466173C2D87D18EFF74A4477C3");
        assert(BaseEncoding::Base64Encode(digest) == "IIgulcTdDPGyvD6E6V58FGXXHUZhc8LYfRjv90pEd8M=");
    }
    cout << "Test SM3Hash Pass" << endl;
    {
        auto f = fopen("./cmake_install.cmake", "r");
        auto digest = SMX::SM3HashFile(f);
        fclose(f);
        cout << "digest(HEX): " << BaseEncoding::Base16Encode(digest) << endl;
    }
    cout << "Test SM3HashFile Pass" << endl;
    const string key = "123456";
    {
        const string str = "A";
        auto digest = SMX::SM3HMAC(str, key);
        assert(BaseEncoding::Base64Encode(digest) == "fPTNHPdW55A6HBAQxH24QeM1aYqifJeJg5bvqQd+280=");
    }
    cout << "Test SM3HMAC Pass" << endl;
    {
        auto f = fopen("./cmake_install.cmake", "r");
        auto digest = SMX::SM3HMACFile(f, key);
        fclose(f);
        cout << "digest(HEX): " << BaseEncoding::Base16Encode(digest) << endl;
    }
    cout << "Test SM3HMACFile Pass" << endl;
}

void TestSM4() {
    const string key = "207CF410532F92A4";
    const string iv = "EA44EBD043D018FB";
    {
        const string str = "A";
        auto cipher = SMX::SM4CBCEncrypt(str, key, iv);
        assert(BaseEncoding::Base64Encode(cipher) == "x47/guzD5nMExu5I1g+08g==");
        auto plain = SMX::SM4CBCDecrypt(cipher, key, iv);
        assert(plain == str);
    }
    {
        const string str = "ABCD";
        auto cipher = SMX::SM4CBCEncrypt(str, key, iv);
        assert(BaseEncoding::Base64Encode(cipher) == "T3uZVtGJcOFu1srjx5rBqQ==");
        auto plain = SMX::SM4CBCDecrypt(cipher, key, iv);
        assert(plain == str);
    }
    {
        const string str = "ABCDABCDABCDABCD";
        auto cipher = SMX::SM4CBCEncrypt(str, key, iv);
        assert(BaseEncoding::Base64Encode(cipher) == "Fvu+KdRANBsSpGjScn9jTCuZZmgv/lwjroN45zVZ+2M=");
        auto plain = SMX::SM4CBCDecrypt(cipher, key, iv);
        assert(plain == str);
    }
    cout << "Test SM4CBC Pass" << endl;
    {
        const string str = "A";
        auto cipher = SMX::SM4CTREncrypt(str, key, iv);
        assert(BaseEncoding::Base64Encode(cipher) == "8A==");
        auto plain = SMX::SM4CTRDecrypt(cipher, key, iv);
        assert(plain == str);
    }
    {
        const string str = "ABCD";
        auto cipher = SMX::SM4CTREncrypt(str, key, iv);
        assert(BaseEncoding::Base64Encode(cipher) == "8H3zyQ==");
        auto plain = SMX::SM4CTRDecrypt(cipher, key, iv);
        assert(plain == str);
    }
    {
        const string str = "ABCDABCDABCDABCD";
        auto cipher = SMX::SM4CTREncrypt(str, key, iv);
        assert(BaseEncoding::Base64Encode(cipher) == "8H3zydZHmu0cdaRVTKqdMg==");
        auto plain = SMX::SM4CTRDecrypt(cipher, key, iv);
        assert(plain == str);
    }
    cout << "Test SM4CTR Pass" << endl;
    const string aad = "1234567812345678";
    {
        const string str = "A";
        auto cipher = SMX::SM4GCMEncrypt(str, key, iv, aad);
        auto plain = SMX::SM4GCMDecrypt(cipher, key, iv, aad);
        assert(plain == str);
    }
    {
        const string str = "ABCD";
        auto cipher = SMX::SM4GCMEncrypt(str, key, iv, aad);
        auto plain = SMX::SM4GCMDecrypt(cipher, key, iv, aad);
        assert(plain == str);
    }
    {
        const string str = "ABCDABCDABCDABCD";
        auto cipher = SMX::SM4GCMEncrypt(str, key, iv, aad);
        auto plain = SMX::SM4GCMDecrypt(cipher, key, iv, aad);
        assert(plain == str);
    }
    cout << "Test SM4GCM Pass" << endl;
    {
        const string str = "A";
        auto cipher = SMX::SM4CBCAndSM3HMACDecrypt(str, key, iv, aad);
        auto plain = SMX::SM4CBCAndSM3HMACDecrypt(cipher, key, iv, aad);
        assert(plain != str);
    }
    {
        const string str = "ABCD";
        auto cipher = SMX::SM4CBCAndSM3HMACDecrypt(str, key, iv, aad);
        auto plain = SMX::SM4CBCAndSM3HMACDecrypt(cipher, key, iv, aad);
        assert(plain != str);
    }
    {
        const string str = "ABCDABCDABCDABCD";
        auto cipher = SMX::SM4CBCAndSM3HMACDecrypt(str, key, iv, aad);
        auto plain = SMX::SM4CBCAndSM3HMACDecrypt(cipher, key, iv, aad);
        assert(plain != str);
    }
    cout << "Test SM4CBC + SM3HMAC Pass" << endl;
    {
        const string str = "A";
        auto cipher = SMX::SM4CTRAndSM3HMACDecrypt(str, key, iv, aad);
        auto plain = SMX::SM4CTRAndSM3HMACDecrypt(cipher, key, iv, aad);
        assert(plain != str);
    }
    {
        const string str = "ABCD";
        auto cipher = SMX::SM4CTRAndSM3HMACDecrypt(str, key, iv, aad);
        auto plain = SMX::SM4CTRAndSM3HMACDecrypt(cipher, key, iv, aad);
        assert(plain != str);
    }
    {
        const string str = "ABCDABCDABCDABCD";
        auto cipher = SMX::SM4CTRAndSM3HMACDecrypt(str, key, iv, aad);
        auto plain = SMX::SM4CTRAndSM3HMACDecrypt(cipher, key, iv, aad);
        assert(plain != str);
    }
    cout << "Test SM4CTR + SM3HMAC Pass" << endl;
}

int main() {
    TestSM3();
    TestSM4();
    return 0;
}