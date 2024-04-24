#include "BaseX.h"
#include "URL.h"

#include <assert.h>

using namespace std;
using namespace Scorpion;

void TestBase16() {
    {

        string str = "Hello, World!";
        string encoded = BaseEncoding::Base16Encode(str);
        assert(encoded == "48656C6C6F2C20576F726C6421");
        string decoded = BaseEncoding::Base16Decode(encoded);
        assert(decoded == str);
    }
    {
        string str = "";
        string encoded = BaseEncoding::Base16Encode(str);
        assert(encoded == "");
        string decoded = BaseEncoding::Base16Decode(encoded);
        assert(decoded == str);
    }
    {
        string str = "A";
        string encoded = BaseEncoding::Base16Encode(str);
        assert(encoded == "41");
        string decoded = BaseEncoding::Base16Decode(encoded);
        assert(decoded == str);
    }
    printf("TestBase16 passed\n");
}

void TestBase32() {
    {
        string str = "Hello, World!";
        string encoded = BaseEncoding::Base32Encode(str);
        assert(encoded == "JBSWY3DPFQQFO33SNRSCC===");
        string decoded = BaseEncoding::Base32Decode(encoded);
        assert(decoded == str);
    }
    {
        string str = "";
        string encoded = BaseEncoding::Base32Encode(str);
        assert(encoded == "");
        string decoded = BaseEncoding::Base32Decode(encoded);
        assert(decoded == str);
    }
    {
        string str = "A";
        string encoded = BaseEncoding::Base32Encode(str);
        assert(encoded == "IE======");
        string decoded = BaseEncoding::Base32Decode(encoded);
        assert(decoded == str);
    }
    printf("TestBase32 passed\n");
}

void TestBase64() {
    {
        string str = "Hello, World!";
        string encoded = BaseEncoding::Base64Encode(str);
        assert(encoded == "SGVsbG8sIFdvcmxkIQ==");
        string decoded = BaseEncoding::Base64Decode(encoded);
        assert(decoded == str);
    }
    {
        string str = "";
        string encoded = BaseEncoding::Base64Encode(str);
        assert(encoded == "");
        string decoded = BaseEncoding::Base64Decode(encoded);
        assert(decoded == str);
    }
    {
        string str = "A";
        string encoded = BaseEncoding::Base64Encode(str);
        assert(encoded == "QQ==");
        string decoded = BaseEncoding::Base64Decode(encoded);
        assert(decoded == str);
    }
    printf("TestBase64 passed\n");
}

void TestURL() {
    {
        string str = "Hello, World!+";
        string encoded = URLEncoding::URLEncode(str);
        assert(encoded == "Hello%2C+World%21%2B");
        string decoded = URLEncoding::URLDecode(encoded);
        assert(decoded == str);
    }
    {
        string str = "";
        string encoded = URLEncoding::URLEncode(str);
        assert(encoded == "");
        string decoded = URLEncoding::URLDecode(encoded);
        assert(decoded == str);
    }
    {
        string str = "A";
        string encoded = URLEncoding::URLEncode(str);
        assert(encoded == "A");
        string decoded = URLEncoding::URLDecode(encoded);
        assert(decoded == str);
    }
    printf("TestURL passed\n");
}

int main() {
    TestBase16();
    TestBase32();
    TestBase64();
    TestURL();
    return 0;
}