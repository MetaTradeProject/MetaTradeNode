#pragma once
#include "BlockChainService.h"
class BlockChainImpl : public metatradenode::BlockChainService
{
public:
	BlockChainImpl():metatradenode::BlockChainService() {};
private:
	virtual void Init(webstomppp::StompFrame& msg);
	virtual void onTrade(webstomppp::StompFrame& msg) override;
	virtual void onSpawn(webstomppp::StompFrame& msg) override;
	virtual void onJudge(webstomppp::StompFrame& msg) override;
	virtual void onSemiSync(webstomppp::StompFrame& msg) override;
	virtual void onSync(webstomppp::StompFrame& msg) override;
};

