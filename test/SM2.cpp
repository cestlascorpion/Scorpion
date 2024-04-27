#include "BaseX.h"
#include "SMX.h"

#include <cassert>
#include <iostream>

using namespace std;
using namespace Scorpion;

void TestSM2() {
    const string password = "123456";
    {
        auto pub = fopen("sm2.pub", "w");
        if (pub == nullptr) {
            cerr << "Failed to open public.pem" << endl;
            return;
        }
        auto pem = fopen("sm2.pem", "w");
        if (pem == nullptr) {
            cerr << "Failed to open private.pem" << endl;
            return;
        }
        auto ret = SMX::SM2KeyGen(password, pub, pem);
        fclose(pub);
        fclose(pem);
        assert(ret == 0 || "Failed to generate SM2 key pair");
        cout << "SM2 key pair generated successfully" << endl;
    }
    {
        const string message = "Hello, World!";
        auto pem = fopen("sm2.pem", "r");
        if (pem == nullptr) {
            cerr << "Failed to open private.pem" << endl;
            return;
        }
        auto signature = SMX::SM2Sign(message, pem, password, "abcdefg");
        fclose(pem);
        assert(!signature.empty() || "Failed to sign message");
        cout << "Message signed successfully" << endl;

        auto pub = fopen("sm2.pub", "r");
        if (pub == nullptr) {
            cerr << "Failed to open public.pem" << endl;
            return;
        }
        auto ret = SMX::SM2Verify(message, signature, pub, "abcdefg");
        fclose(pub);
        assert(ret == 0 || "Failed to verify signature");
        cout << "Signature verified successfully" << endl;
    }
    {
        const string message = "Hello, World!";
        auto pub = fopen("sm2.pub", "r");
        if (pub == nullptr) {
            cerr << "Failed to open public.pem" << endl;
            return;
        }
        auto encrypted = SMX::SM2Encrypt(message, pub);
        fclose(pub);
        assert(!encrypted.empty() || "Failed to encrypt message");
        cout << "Message encrypted successfully" << endl;

        auto pem = fopen("sm2.pem", "r");
        if (pem == nullptr) {
            cerr << "Failed to open private.pem" << endl;
            return;
        }
        auto decrypted = SMX::SM2Decrypt(encrypted, pem, password);
        fclose(pem);
        assert(decrypted == message || "Failed to decrypt message");
        cout << "Message decrypted successfully" << endl;
    }
}

int main() {
    TestSM2();
    return 0;
}