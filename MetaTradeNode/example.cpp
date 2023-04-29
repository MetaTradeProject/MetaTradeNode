#include "BlockChainService.h"
#include "MetaTradeClient.h"
#include "BlockChainImpl.h"
#include "CryptoUtils.h"

int main() {
	/*metatradenode::MetaTradeClient client("123");
	BlockChainImpl block_chain;
	client.RegisterService(&block_chain);
	block_chain.RegisterClient(&client);
	
	client.RunSync();*/
	std::string str = "a665a45920422f9d417e4867efdc2ef1";
	std::cout << CryptoUtils::PrivateKey2Address(str);
}