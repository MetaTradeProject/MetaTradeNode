#include "CryptoUtils.h"

std::string CryptoUtils::GetSha256(std::string& src)
{
    return picosha2::hash256_hex_string(src);
}

std::string CryptoUtils::GetSha256(const char* src)
{
    return picosha2::hash256_hex_string(std::string(src));
}

std::string CryptoUtils::PrivateKey2Address(unsigned char* src)
{
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

    return std::string();
}

std::string CryptoUtils::PrivateKey2Address(std::string& src)
{
    unsigned char seckey[32];
    if (src.size() != 32) {
        return "";
    }
    else {
        for (size_t i = 0; i < 32; i++) {
            seckey[i] = (unsigned char)src[i];
        }
        return CryptoUtils::PrivateKey2Address(seckey);
    }
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
