#include "CryptoUtils.h"
#include <iostream>

static const char* pszBase58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
static const int8_t mapBase58[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1, 0, 1, 2, 3, 4, 5, 6,  7, 8,-1,-1,-1,-1,-1,-1,
    -1, 9,10,11,12,13,14,15, 16,-1,17,18,19,20,21,-1,
    22,23,24,25,26,27,28,29, 30,31,32,-1,-1,-1,-1,-1,
    -1,33,34,35,36,37,38,39, 40,41,42,43,-1,44,45,46,
    47,48,49,50,51,52,53,54, 55,56,57,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
};

std::string CryptoUtils::GetSha256(const char* src) {
    return picosha2::hash256_hex_string(std::string(src));
}

std::string CryptoUtils::PrivateKey2Address(unsigned char* src) {
    unsigned char randomize[32];
    int return_val;
    secp256k1_pubkey pubkey;
    unsigned char compressed_pubkey[33];
    size_t len;
    secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_NONE);

    if (!fill_random(randomize, sizeof(randomize))) {
        printf("Failed to generate randomness\n");
        return "";
    }
    return_val = secp256k1_context_randomize(ctx, randomize);
    assert(return_val);

    if (!secp256k1_ec_seckey_verify(ctx, src)){
        return "";
    }

    /* Public key creation using a valid context with a verified secret key should never fail */
    return_val = secp256k1_ec_pubkey_create(ctx, &pubkey, src);
    assert(return_val);

    /* Serialize the pubkey in a compressed form(33 bytes). Should always return 1. */
    len = sizeof(compressed_pubkey);
    return_val = secp256k1_ec_pubkey_serialize(ctx, compressed_pubkey, &len, &pubkey, SECP256K1_EC_COMPRESSED);
    assert(return_val);
    /* Should be the same size as the size of the output, because we passed a 33 byte array. */
    assert(len == sizeof(compressed_pubkey));

    //Hash160
    picosha2::hash256_one_by_one hasher;
    hasher.process(std::begin(compressed_pubkey), std::end(compressed_pubkey));
    hasher.finish();
    std::vector<unsigned char> sha256(picosha2::k_digest_size);
    hasher.get_hash_bytes(sha256.begin(), sha256.end());
    unsigned char raw[32];
    std::copy(sha256.begin(), sha256.end(), raw);

    dword MDbuf[5];
    unsigned char rmd_hash[20];
    MDinit(MDbuf);
    MDfinish(MDbuf, raw, 32, 0);
    for (size_t i = 0; i < 20; i += 4) {
        rmd_hash[i] = MDbuf[i >> 2];         /* implicit cast to byte  */
        rmd_hash[i + 1] = (MDbuf[i >> 2] >> 8);  /*  extracts the 8 least  */
        rmd_hash[i + 2] = (MDbuf[i >> 2] >> 16);  /*  significant bits.     */
        rmd_hash[i + 3] = (MDbuf[i >> 2] >> 24);
    }

    char data[43] = {};
    data[0] = metatradenode::NETWORK_VERSION[0];
    data[1] = metatradenode::NETWORK_VERSION[1];
    for (int j = 0; j < 20; j++)
    {
        char c[5];
        sprintf_s(c, "%02x", rmd_hash[j]);
        strcat_s(data, c);
    }

    std::string check = GetSha256(GetSha256(data).c_str()).substr(0, 8);

    unsigned char cat[25]{};
    cat[0] = (hex2byte(metatradenode::NETWORK_VERSION[0]) << 4) | hex2byte(metatradenode::NETWORK_VERSION[1]);
    for (size_t i = 1; i < 21; i++) {
        cat[i] = rmd_hash[i - 1];
    }
    cat[21] = (hex2byte(check[0]) << 4) | hex2byte(check[1]);
    cat[22] = (hex2byte(check[2]) << 4) | hex2byte(check[3]);
    cat[23] = (hex2byte(check[4]) << 4) | hex2byte(check[5]);
    cat[24] = (hex2byte(check[6]) << 4) | hex2byte(check[7]);

    return EncodeBase58(std::begin(cat), std::end(cat));
}

std::string CryptoUtils::PrivateKey2Address(const char* src) {
    unsigned char seckey[32]{};
    if (strlen(src) != 64) {
        return "";
    }
    else {
        for (size_t i = 0; i < 32; i++){
            unsigned char fir = hex2byte(src[2 * i]);
            unsigned char sec = hex2byte(src[2 * i + 1]);
            seckey[i] = (fir << 4) | sec;
        }
        return CryptoUtils::PrivateKey2Address(seckey);
    }
}

bool CryptoUtils::isValidAddress(const char* address)
{
    if (address[0] != '1') {
        return false;
    }
    std::vector<unsigned char> cat;
    cat.push_back(0x00);
    if (DecodeBase58(address, cat, 24)) {
        unsigned char check[4];
        check[0] = cat[20];
        check[1] = cat[21];
        check[2] = cat[22];
        check[3] = cat[23];

        char data[43]{};
        data[0] = '0';
        data[1] = '0';
        for (int j = 0; j < 20; j++)
        {
            char c[5];
            sprintf_s(c, "%02x", cat[j]);
            strcat_s(data, c);
        }

        std::string judge = GetSha256(GetSha256(data).c_str()).substr(0, 8);
        unsigned char a = (hex2byte(judge[0]) << 4) | hex2byte(judge[1]);
        unsigned char b = (hex2byte(judge[2]) << 4) | hex2byte(judge[3]);
        unsigned char c = (hex2byte(judge[4]) << 4) | hex2byte(judge[5]);
        unsigned char d = (hex2byte(judge[6]) << 4) | hex2byte(judge[7]);

        if (a == check[0] && b == check[1] && c == check[2] && d == check[3]) {
            return true;
        }
    }
    return false;
}

std::string CryptoUtils::EncodeBase58(const unsigned char* pbegin, const unsigned char* pend)
{
    // Skip & count leading zeroes.
    int zeroes = 0;
    int length = 0;
    while (pbegin != pend && *pbegin == 0) {
        pbegin++;
        zeroes++;
    }
    // Allocate enough space in big-endian base58 representation.
    int size = (pend - pbegin) * 138 / 100 + 1; // log(256) / log(58), rounded up.
    std::vector<unsigned char> b58(size);
    while (pbegin != pend) {
        int carry = *pbegin;
        int i = 0;
        // Apply "b58 = b58 * 256 + ch".
        for (auto it = b58.rbegin();
            (carry != 0 || i < length) && (it != b58.rend());
            it++, i++) {
            carry += 256 * (*it);
            *it = carry % 58;
            carry /= 58;
        }

        assert(carry == 0);
        length = i;
        pbegin++;
    }
    // Skip leading zeroes in base58 result.
    std::vector<unsigned char>::iterator it = b58.begin() + (size - length);
    while (it != b58.end() && *it == 0)
        it++;
    // Translate the result into a string.
    std::string str;
    str.reserve(zeroes + (b58.end() - it));
    str.assign(zeroes, '1');
    while (it != b58.end())
        str += pszBase58[*(it++)];
    return str;
}

bool CryptoUtils::DecodeBase58(const char* psz, std::vector<unsigned char>& vch, int max_ret_len)
{
    // Skip and count leading '1's.
    int zeroes = 0;
    int length = 0;
    while (*psz && IsSpace(*psz)) {
        zeroes++;
        if (zeroes > max_ret_len) return false;
        psz++;
    }
    // Allocate enough space in big-endian base256 representation.
    int size = strlen(psz) * 733 / 1000 + 1; // log(58) / log(256), rounded up.
    std::vector<unsigned char> b256(size);
    // Process the characters.
    static_assert(sizeof(mapBase58) / sizeof(mapBase58[0]) == 256, "mapBase58.size() should be 256"); // guarantee not out of range
    while (*psz && !IsSpace(*psz)) {
        // Decode base58 character
        int carry = mapBase58[(uint8_t)*psz];
        if (carry == -1)  // Invalid b58 character
            return false;
        int i = 0;
        for (std::vector<unsigned char>::reverse_iterator it = b256.rbegin(); (carry != 0 || i < length) && (it != b256.rend()); ++it, ++i) {
            carry += 58 * (*it);
            *it = carry % 256;
            carry /= 256;
        }
        assert(carry == 0);
        length = i;
        if (length + zeroes > max_ret_len) return false;
        psz++;
    }
    // Skip trailing spaces.
    while (IsSpace(*psz))
        psz++;
    if (*psz != 0)
        return false;
    // Skip leading zeroes in b256.
    std::vector<unsigned char>::iterator it = b256.begin() + (size - length);
    // Copy result into output vector.
    vch.reserve(zeroes + (b256.end() - it));
    vch.assign(zeroes, 0x00);
    while (it != b256.end())
        vch.push_back(*(it++));
    return true;
}

int CryptoUtils::fill_random(unsigned char* data, size_t size) {
#if defined(_WIN32)
    NTSTATUS res = BCryptGenRandom(NULL, data, size, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (res != STATUS_SUCCESS || size > ULONG_MAX) {
        return 0;
    }
    else {
        return 1;
    }
#elif defined(__linux__) || defined(__FreeBSD__)
    /* If `getrandom(2)` is not available you should fallback to /dev/urandom */
    ssize_t res = getrandom(data, size, 0);
    if (res < 0 || (size_t)res != size) {
        return 0;
    }
    else {
        return 1;
    }
#elif defined(__APPLE__) || defined(__OpenBSD__)
    /* If `getentropy(2)` is not available you should fallback to either
     * `SecRandomCopyBytes` or /dev/urandom */
    int res = getentropy(data, size);
    if (res == 0) {
        return 1;
    }
    else {
        return 0;
    }
#endif
    return 0;
}

unsigned char CryptoUtils::hex2byte(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return 0;
}
