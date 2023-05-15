#include "BlockChainService.h"
using namespace metatradenode;
void BlockChainService::Init(webstomppp::StompCallbackMsg msg) { 
	
	_client->RegisterSubscribe(); 
}