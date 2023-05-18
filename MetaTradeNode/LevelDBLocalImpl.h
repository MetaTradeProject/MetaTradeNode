#pragma once
#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <atomic>
#include <string>
#include "LocalService.h"
#include "MetaTradeBlockchainImpl.h"


namespace metatradenode {
	extern const char* BORADCAST_ADDRESS;
};

class LevelDBLocalImpl : public metatradenode::LocalService {
private:
	leveldb::DB* _db{ nullptr };
	static std::string Key(std::string address, std::string item_id) { return address.append("-").append(item_id); };
	std::atomic<int> _cur_index { 0 };
	void updateIndexLocal();
public:
	LevelDBLocalImpl();
	~LevelDBLocalImpl() {
		if (_db != nullptr) {
			updateIndexLocal();
			delete _db;
		}
	};
	void onLocalSync(std::vector<metatradenode::Block>&) override;
	int getStartIndex() override { return _cur_index.load(); };

	bool isBalanceTrade(metatradenode::Trade) override;

	long long queryAmount(std::string address, std::string item_id);
};

