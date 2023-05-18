#include "BlockchainService.h"
#include "MetaTradeClient.h"
#include "MetaTradeBlockchainImpl.h"
#include "CryptoUtils.h"
#include <cJSON/cJSON.h>
#include <webstomp++/WebStompType.h>
#include "MetaTradeNode.h"
#pragma comment(lib , "cJSON.lib")

void view(std::string);
int main() {
	/*std::string str = "8F72F6B29E6E225A36B68DFE333C7CE5E55D83249D3D2CD6332671FA445C4DD3";
	std::cout << "Private Key: " << str << std::endl;
	char* pub;
	char* address;
	CryptoUtils::GeneratePublic(str.c_str(), pub, address);
	std::cout << "Public Key: " << pub << std::endl;
	std::cout << "Address: " << address << std::endl;
	std::cout << "isValid Address: " << CryptoUtils::isValidAddress(address) << std::endl;
	char* sign;
	const char* data_hash = "03ac674216f3e15c761ee1a5e255f067953623c8b388b4459e13f978d7c846f4";
	CryptoUtils::SignTrade(data_hash, str.c_str(), sign);
	std::cout << "Message Hash: " << data_hash << std::endl;
	std::cout << "Signature: " << sign << std::endl;
	std::cout << "isValidTrade1: " << CryptoUtils::isValidSignature("03ac674216f3e15c761ee1a5e255f067953623c8b388b4459e13f978d7c846f4",
		"b0745147017bd3458e6c08b8b2d194f4e16c96d59f42229394095e74e291b0165da374c7e8e11565bd428541a92c012c973917adf4d013fd15974d4bccd7a3e9", 
		"0259dee66ab619c4a9e215d070052d1ae3a2075e5f58c67516b2e4884a88c79be9") << std::endl;
	std::cout << "cJSON Ver: " << cJSON_Version() << std::endl;*/

	metatradenode::nodeconfig cfg{};
	strcpy_s(cfg.prikey, 65, "123");
	strcpy_s(cfg.pubkey, 67, "123");
	strcpy_s(cfg.address, 35, "123");
	cfg.mining = true;
	metatradenode::MetaTradeNode node(cfg);
	node.init();
	node.run(false);

	Sleep(30000);

	std::cout << node.queryAmount("123", "0") << std::endl;

	//view("metatradelocal");
	return 0;
}