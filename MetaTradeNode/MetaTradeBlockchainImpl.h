#pragma once
#include "BlockChainService.h"
#include "MetaTradeBlockchainModels.h"
#include <vector>
#include <deque>
#include <cJSON/cJSON.h>
#include <shared_mutex>
#include <atomic>
#include <condition_variable>

class LevelDBLocalImpl;
extern constexpr const char* PropertyKey();

class MetaTradeBlockchainImpl : public metatradenode::BlockchainService
{
public:
	MetaTradeBlockchainImpl():metatradenode::BlockchainService() {};
	void SendTrade(metatradenode::Trade& trade) override;
	friend class LevelDBLocalImpl;
private:
	std::vector<metatradenode::Block> _chain;
	std::deque<metatradenode::RawBlock> _rawblock_deque;
	std::vector<metatradenode::Trade> _trade_list;
	std::string _wallet_address;
	std::shared_mutex _lock;
	std::condition_variable _cond;
	std::atomic<bool> _proof_done{false};

	void ParseSyncMessage(const char* raw);
	void ParseSemiSyncMessage(const char* raw, metatradenode::Block& block);

	bool isValidProof(int proof, metatradenode::RawBlock& raw_block);
	void SendProofMessage(metatradenode::Block& block);
	void SendAgreeMessage(int proof);
	void SendSyncRequest();
	bool isValidTrade(metatradenode::Trade& trade);

	void MiningBlock();

	static void ParseChain(cJSON* root, std::vector<metatradenode::Block>& chain);
	static void ParseRawBlocks(cJSON* root, std::deque<metatradenode::RawBlock>& raw_blocks);
	static void ParseTradeList(cJSON* root, std::vector<metatradenode::Trade>& vec);
	static void ParseTrade(cJSON* root, metatradenode::Trade& trade);
	static void ParseRawBlock(cJSON* root, metatradenode::RawBlock& raw_block);
	static void ParseBlock(cJSON* root, metatradenode::Block& block);

	virtual void Init(webstomppp::StompCallbackMsg msg);
	virtual void onTrade(webstomppp::StompCallbackMsg msg) override;
	virtual void onSpawn(webstomppp::StompCallbackMsg msg) override;
	virtual void onJudge(webstomppp::StompCallbackMsg msg) override;
	virtual void onSemiSync(webstomppp::StompCallbackMsg msg) override;
	virtual void onSync(webstomppp::StompCallbackMsg msg) override;
	virtual void Stop() override;
};

