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
	std::cout << CryptoUtils::GetSha256("123");
}