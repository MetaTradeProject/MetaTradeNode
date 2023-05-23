#pragma once
#include "MetaTradeBlockchainModels.h"
namespace metatradenode {
	//only focus on chain, instead of rawblocks and tradelist
	class LocalService {
	public:
		virtual int getStartIndex() { return 0; };
		virtual bool isBalanceTrade(metatradenode::Trade) = 0;
		virtual void onLocalSync(std::vector<metatradenode::Block>&) = 0; // Semisync or sync ? only sync blockchain
		virtual long long queryAmount(std::string address, std::string item_id) = 0;
		virtual void queryBills(std::string address, std::vector<metatradenode::Bill>& bills) = 0;
		virtual ~LocalService() {};
	};
}