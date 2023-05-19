#pragma once
#include "BlockChainService.h"
#include "MetaTradeBlockchainModels.h"
#include <vector>
#include <deque>
#include <cJSON/cJSON.h>
#include <shared_mutex>
#include <atomic>
#include <condition_variable>
#include <thread>

class LevelDBLocalImpl;
extern constexpr const char* PropertyKey();

namespace metatradenode {
	extern const char* BORADCAST_ADDRESS;
};

class MetaTradeBlockchainImpl : public metatradenode::BlockchainService
{
public:
	MetaTradeBlockchainImpl(std::string address):metatradenode::BlockchainService(), _wallet_address(address) {};
	~MetaTradeBlockchainImpl();
	void SendTrade(metatradenode::Trade& trade) override;
	void Mining() override;
	void SendSyncRequest() override;
	long long queryAmount(std::string address, std::string item_id) override;
	long long queryTransitAmount(std::string address, std::string item_id) override;
	friend class LevelDBLocalImpl;
private:
	std::vector<metatradenode::Block> _chain {};
	std::deque<metatradenode::RawBlock> _rawblock_deque {};
	std::vector<metatradenode::Trade> _trade_list {};
	std::string _wallet_address;
	std::mutex _lock {};
	std::condition_variable _cond {};
	std::thread* _mining_thread { nullptr };
	std::atomic<bool> _proof_done { false };
	std::atomic<bool> _quit_flag { false };

	void ParseSyncMessage(const char* raw);
	void ParseSemiSyncMessage(const char* raw, metatradenode::Block& block);

	bool isValidProof(int proof, metatradenode::RawBlock& raw_block);
	void SendProofMessage(metatradenode::Block& block);
	void SendAgreeMessage(int proof);
	bool isValidTrade(metatradenode::Trade& trade);

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

	void MiningBlock();
};

