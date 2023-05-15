#pragma once
#include "MetaTradeBlockchainModels.h"
namespace metatradenode {
	//only focus on chain, instead of rawblocks and tradelist
	class LocalService {
	public:
		virtual int getStartIndex() { return 0; };
		virtual bool isBalanceTrade(metatradenode::Trade) = 0;
		virtual void onLocalSync(const char*) = 0; // Semisync or sync ? only sync blockchain
		virtual void onSemiSync(const char*) = 0; // only parse new block
	};
}