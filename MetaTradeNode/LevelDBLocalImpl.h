#pragma once
#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <atomic>
#include "LocalService.h"
#include "MetaTradeBlockchainImpl.h"


class LevelDBLocalImpl : public metatradenode::LocalService {
private:
	leveldb::DB* _db{ nullptr };
	static std::string Key(std::string address, std::string item_id) { return address.append("-").append(item_id); };
	std::atomic<int> _cur_index { 0 };
public:
	LevelDBLocalImpl();
	~LevelDBLocalImpl() {
		if (_db != nullptr) {
			auto s = _db->Put(leveldb::WriteOptions(), PropertyKey(), std::to_string(_cur_index));
			assert(s.ok());
			delete _db;
		}
	};
	void onLocalSync(std::vector<metatradenode::Block>&) override;
	void onSemiSync(const metatradenode::Block&) override;
	int getStartIndex() override { return _cur_index.load(); };

	bool isBalanceTrade(metatradenode::Trade) override;

	long queryAmount(std::string address, std::string item_id);
};
