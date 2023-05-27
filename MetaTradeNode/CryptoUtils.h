#pragma once
#include <crypto/picosha2.h>
#include <crypto/rmd160.h>
#include <stddef.h>
#include <climits>
#include <cstdio>
#include <cstring>
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
#pragma comment(lib , "secp256k1.lib")
#pragma comment(lib , "rmd160.lib")
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#include <sys/random.h>
#elif defined(__OpenBSD__)
#include <unistd.h>
#else
#error "Couldn't identify the OS"
#endif


#if !defined(__WINDOWS__) && (defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32))
#define __WINDOWS__
#endif

#ifdef __WINDOWS__
#define META_NODE_PUBLIC   __declspec(dllexport)
#else
#define META_NODE_PUBLIC 
#endif


namespace metatradenode {
	extern const char* NETWORK_VERSION;
}

class META_NODE_PUBLIC CryptoUtils {
	/*
		Private key: random --> sha256(random)
	*/
public:
	static std::string GetSha256(const char* src);
	static void GeneratePublic(const char* private_key, char*& public_key, char*& address);
	static bool isValidAddress(const char* address);
	static bool isValidSignature(const char* trade_hash, const char* signature, const char* sender_public_key);
	static void SignTrade(const char* trade_hash, const char* private_key, char*& signature);
private:
	static bool IsSpace(char c) { return c == ' '; };
	static std::string EncodeBase58(const unsigned char* pbegin, const unsigned char* pend);
	static bool DecodeBase58(const char* psz, std::vector<unsigned char>& vch, int max_ret_len);
	static int fill_random(unsigned char* data, size_t size);
	static void SecpECDSA(unsigned char* src, char*& public_key, char*& address);
	static unsigned char hex2byte(char c);
};