#include "BlockChainImpl.h"

void BlockChainImpl::Init(webstomppp::StompFrame& msg)
{
	std::cout << "init\n" << msg.body << std::endl;
	BlockChainService::Init(msg);
}

void BlockChainImpl::onTrade(webstomppp::StompFrame& msg)
{
	std::cout << "trade\n";
}

void BlockChainImpl::onSpawn(webstomppp::StompFrame& msg)
{
	std::cout << "spawn\n";
}

void BlockChainImpl::onJudge(webstomppp::StompFrame& msg)
{
	std::cout << "judge\n";
}

void BlockChainImpl::onSemiSync(webstomppp::StompFrame& msg)
{
	std::cout << "semi\n";
}

void BlockChainImpl::onSync(webstomppp::StompFrame& msg)
{
	std::cout << "sync\n";
}
