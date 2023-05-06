#pragma once
#include "BlockChainService.h"
class BlockChainImpl : public metatradenode::BlockChainService
{
public:
	BlockChainImpl():metatradenode::BlockChainService() {};
private:
	virtual void Init(webstomppp::StompCallbackMsg msg);
	virtual void onTrade(webstomppp::StompCallbackMsg msg) override;
	virtual void onSpawn(webstomppp::StompCallbackMsg msg) override;
	virtual void onJudge(webstomppp::StompCallbackMsg msg) override;
	virtual void onSemiSync(webstomppp::StompCallbackMsg msg) override;
	virtual void onSync(webstomppp::StompCallbackMsg msg) override;
	virtual void Stop() override;
};

