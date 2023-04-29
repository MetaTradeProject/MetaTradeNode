#include <crypto/picosha2.h>

class CryptoUtils {
public:
	static std::string GetSha256(std::string& src);
	static std::string GetSha256(const char* src);
};