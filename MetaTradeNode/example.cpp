#include "BlockChainService.h"
#include "MetaTradeClient.h"
#include "BlockChainImpl.h"
#include "CryptoUtils.h"
#include <cJSON/cJSON.h>
#pragma comment(lib , "cJSON.lib")


int main() {
	std::string str = "8F72F6B29E6E225A36B68DFE333C7CE5E55D83249D3D2CD6332671FA445C4DD3";
	std::string address = CryptoUtils::PrivateKey2Address(str);
	std::cout << address << std::endl;
	std::cout << CryptoUtils::isValidAddress(address) << std::endl;
	std::cout << cJSON_Version() << std::endl;
	metatradenode::MetaTradeClient client("123");
	BlockChainImpl block_chain;
	client.RegisterService(&block_chain);
	block_chain.RegisterClient(&client);
	
	client.RunSync();
}