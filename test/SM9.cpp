#include "BaseX.h"
#include "SMX.h"

#include <cassert>
#include <iostream>

using namespace std;
using namespace Scorpion;

void TestSM9() {
    const string masterPass = "123456";
    const string userPass = "1234";
    const string identity = "identity";
    {
        auto mpub = fopen("sm9sign.master.pub", "w");
        if (mpub == nullptr) {
            cout << "Failed to open master public.pem" << endl;
            return;
        }
        auto mpem = fopen("sm9sign.master.pem", "w");
        if (mpem == nullptr) {
            cout << "Failed to open master private.pem" << endl;
            return;
        }
        auto ret = SMX::SM9SignMasterKeyGen(masterPass, mpub, mpem);
        fclose(mpub);
        fclose(mpem);
        assert(ret == 0 || "Failed to generate SM9 sign master key pair");
        cout << "SM9 sign master key pair generated successfully" << endl;

        {
            mpem = fopen("sm9sign.master.pem", "r");
            if (mpem == nullptr) {
                cout << "Failed to open master private.pem" << endl;
                return;
            }
            auto upem = fopen("sm9sign.user.pem", "w");
            if (upem == nullptr) {
                cout << "Failed to open user private.pem" << endl;
                return;
            }
            ret = SMX::SM9SignUserKeyGen(masterPass, mpem, userPass, upem, identity);
            fclose(mpem);
            fclose(upem);
            assert(ret == 0 || "Failed to generate SM9 sign user key pair");
            cout << "SM9 sign user key pair generated successfully" << endl;

            {
                upem = fopen("sm9sign.user.pem", "r");
                if (upem == nullptr) {
                    cout << "Failed to open user private.pem" << endl;
                    return;
                }
                auto sigature = SMX::SM9Sign("Hello, world!", upem, userPass);
                fclose(upem);
                assert(sigature.size() > 0 || "Failed to sign message");
                cout << "Message signed successfully" << endl;

                mpub = fopen("sm9sign.master.pub", "r");
                if (mpub == nullptr) {
                    cout << "Failed to open master public.pem" << endl;
                    return;
                }
                auto verify = SMX::SM9Verify("Hello, world!", sigature, mpub, identity);
                fclose(mpub);
                assert(verify == 0 || "Failed to verify message");
                cout << "Message verified successfully" << endl;
            }
        }
    }
    {
        auto mpub = fopen("sm9enc.master.pub", "w");
        if (mpub == nullptr) {
            cout << "Failed to open master public.pem" << endl;
            return;
        }
        auto mpem = fopen("sm9enc.master.pem", "w");
        if (mpem == nullptr) {
            cout << "Failed to open master private.pem" << endl;
            return;
        }
        auto ret = SMX::SM9EncryptMasterKeyGen(masterPass, mpub, mpem);
        fclose(mpub);
        fclose(mpem);
        assert(ret == 0 || "Failed to generate SM9 encrypt master key pair");
        cout << "SM9 encrypt master key pair generated successfully" << endl;

        {
            mpem = fopen("sm9enc.master.pem", "r");
            if (mpem == nullptr) {
                cout << "Failed to open master private.pem" << endl;
                return;
            }
            auto upem = fopen("sm9enc.user.pem", "w");
            if (upem == nullptr) {
                cout << "Failed to open user private.pem" << endl;
                return;
            }
            ret = SMX::SM9EncryptUserKeyGen(masterPass, mpem, userPass, upem, identity);
            fclose(mpem);
            fclose(upem);
            assert(ret == 0 || "Failed to generate SM9 encrypt user key pair");
            cout << "SM9 encrypt user key pair generated successfully" << endl;

            {
                mpub = fopen("sm9enc.master.pub", "r");
                if (mpub == nullptr) {
                    cout << "Failed to open master public.pem" << endl;
                    return;
                }
                auto ciphertext = SMX::SM9Encrypt("Hello, world!", mpub, identity);
                fclose(mpub);
                assert(ciphertext.size() > 0 || "Failed to encrypt message");
                cout << "Message encrypted successfully" << endl;

                upem = fopen("sm9enc.user.pem", "r");
                if (upem == nullptr) {
                    cout << "Failed to open user private.pem" << endl;
                    return;
                }
                auto plaintext = SMX::SM9Decrypt(ciphertext, userPass, upem, identity);
                fclose(upem);
                assert(plaintext.size() > 0 || "Failed to decrypt message");
                assert(plaintext == "Hello, world!" || "Failed to decrypt message");
                cout << "Message decrypted successfully" << endl;
            }
        }
    }
}

int main() {
    TestSM9();
    return 0;
}