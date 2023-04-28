#include "BlockChainService.h"
#include "MetaTradeClient.h"
#include "BlockChainImpl.h"

int main() {
	metatradenode::MetaTradeClient client("123");
	BlockChainImpl block_chain;
	client.RegisterService(&block_chain);
	block_chain.RegisterClient(&client);
	
	client.RunSync();
}