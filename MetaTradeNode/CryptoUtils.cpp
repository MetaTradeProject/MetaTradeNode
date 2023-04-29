#include "CryptoUtils.h"

std::string CryptoUtils::GetSha256(std::string& src)
{
    return picosha2::hash256_hex_string(src);
}

std::string CryptoUtils::GetSha256(const char* src)
{
    return picosha2::hash256_hex_string(std::string(src));
}
