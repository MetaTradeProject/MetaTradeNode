#include "BlockChainImpl.h"

void BlockChainImpl::Init(webstomppp::StompCallbackMsg msg)
{
	std::cout << "init\n" << msg.body << std::endl;
	BlockChainService::Init(msg);
}

void BlockChainImpl::onTrade(webstomppp::StompCallbackMsg msg)
{
	std::cout << "trade\n";
}

void BlockChainImpl::onSpawn(webstomppp::StompCallbackMsg msg)
{
	std::cout << "spawn\n";
}

void BlockChainImpl::onJudge(webstomppp::StompCallbackMsg msg)
{
	std::cout << "judge\n";
}

void BlockChainImpl::onSemiSync(webstomppp::StompCallbackMsg msg)
{
	std::cout << "semi\n";
}

void BlockChainImpl::onSync(webstomppp::StompCallbackMsg msg)
{
	std::cout << "sync\n";
}

void BlockChainImpl::Stop() {

}
