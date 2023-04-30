#include "BlockChainService.h"
#include "MetaTradeClient.h"

void metatradenode::BlockChainService::Init(webstomppp::StompCallbackMsg msg)
{
	_client->RegisterSubscribe();
}

void metatradenode::BlockChainService::RegisterClient(metatradenode::MetaTradeClient* client)
{
	this->_client = client;
}
