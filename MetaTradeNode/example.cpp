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
	/*std::string str = "37cb1ebf8767628f40cdc547fb81d9b6e51119622df344fb1f6db6052ea40c8e";
	std::cout << "Private Key: " << str << std::endl;
	char* pub;
	char* address;
	CryptoUtils::GeneratePublic(str.c_str(), pub, address);
	std::cout << "Public Key: " << pub << std::endl;
	std::cout << "Address: " << address << std::endl;
	std::cout << "isValid Address: " << CryptoUtils::isValidAddress(address) << std::endl;

	return 0;*/
	metatradenode::nodeconfig cfg{};
	strcpy_s(cfg.prikey, 65, "37cb1ebf8767628f40cdc547fb81d9b6e51119622df344fb1f6db6052ea40c8e");
	strcpy_s(cfg.pubkey, 67, "033362df4159d66739ad7d551e1691396568f956f6e3543fc5c981a2de9416ce25");
	strcpy_s(cfg.address, 35, "1HtPYxr6vXPwMJbhvHWkUdjkGagJVSdasc");
	cfg.mining = false;
	metatradenode::MetaTradeNode node(cfg);
	node.init();
	node.run(false);

	//Sleep(3000);

	std::cout << node.queryAmount("1HtPYxr6vXPwMJbhvHWkUdjkGagJVSdasc", "0") << std::endl;

	/*node.submitTrade("1DZdHL63ceodz5ZftvhXtiA8KwNCkv2vVL", "0", 10);
	Sleep(2000);*/

	std::cout << node.queryTransitAmount("1HtPYxr6vXPwMJbhvHWkUdjkGagJVSdasc", "0") << std::endl;

	metatradenode::Bill* bills{nullptr};
	uint64_t sz;
	node.queryTransitBills("1HtPYxr6vXPwMJbhvHWkUdjkGagJVSdasc", &bills, &sz);

	std::cout << sz << std::endl;
	//view("local");
	return 0;
}