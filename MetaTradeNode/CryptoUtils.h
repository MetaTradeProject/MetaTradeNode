#include <crypto/picosha2.h>
#include <secp256k1/secp256k1.h>
#if defined(_WIN32)
/*
 * The defined WIN32_NO_STATUS macro disables return code definitions in
 * windows.h, which avoids "macro redefinition" MSVC warnings in ntstatus.h.
 */
#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS
#include <ntstatus.h>
#include <bcrypt.h>
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#include <sys/random.h>
#elif defined(__OpenBSD__)
#include <unistd.h>
#else
#error "Couldn't identify the OS"
#endif

#include <stddef.h>
#include <limits.h>
#include <stdio.h>

#pragma comment(lib , "secp256k1.lib")

class CryptoUtils {
	/*
		Private key: random --> sha256(random)
	*/
public:
	static std::string GetSha256(std::string& src);
	static std::string GetSha256(const char* src);
	static std::string PrivateKey2Address(unsigned char* src);
	static std::string PrivateKey2Address(std::string& src);
private:
	static int fill_random(unsigned char* data, size_t size);
};