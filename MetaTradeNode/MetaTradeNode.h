#pragma once
#include "BlockchainService.h"
#include "LocalService.h"
#include "MetaTradeBlockchainImpl.h"
#include "LevelDBLocalImpl.h"
#include "MetaTradeClient.h"
#include "MetaTradeBlockchainModels.h"
#include "CryptoUtils.h"



namespace metatradenode {
	const long long commission_rate = 100;
	const long long commission_item_fix = 500;

	struct META_NODE_PUBLIC nodeconfig {
		char prikey[65];
		char pubkey[67];
		char address[35];
		bool mining;
		bool force;
		metatradenode::MiningPublisher* publisher;
	};

	class META_NODE_PUBLIC MetaTradeNode {
	private:
		metatradenode::nodeconfig _config;
		metatradenode::MetaTradeClient* _client{ nullptr };
		metatradenode::BlockchainService* _bc_service{ nullptr };
		metatradenode::LocalService* _lc_service{ nullptr };
		void ClearLocalTemp();
	public:
		MetaTradeNode(metatradenode::nodeconfig& config) : _config(config) {};

		MetaTradeNode() = delete;
		MetaTradeNode& operator=(const MetaTradeNode&) = delete;
		MetaTradeNode(const MetaTradeNode&) = delete;
		~MetaTradeNode();

		void init();
		void reload();
		void run(bool sync = true);
		long long queryAmount(const char* address, const char* item_id);
		long long queryTransitAmount(const char* address, const char* item_id);
		void queryBills(const char* address, metatradenode::Bill** bills, uint64_t* sz);
		void queryTransitBills(const char* address, metatradenode::Bill** bills, uint64_t* sz);
		void submitTrade(const char* receiver, const char* item_id, long long amount);
	};

}

