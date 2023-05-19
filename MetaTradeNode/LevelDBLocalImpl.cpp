#include "LevelDBLocalImpl.h"
#include <fstream>

constexpr const char* PropertyKey() { return "start-index"; };
constexpr const char* level_db_name = "local";
constexpr const char* sql_db_name = "LocalBills.db";
constexpr const char* create_tb_sql = "create table bill(sender TEXT, receiver TEXT, id TEXT, amount BIGINT, commission BIGINT, timestamp UNSIGNED BIG INT)";
constexpr const char* insert_td_sql = "insert into bill VALUES ('%s', '%s', '%s', %lld, %lld, %lld)";
constexpr const char* select_td_sql = "select * from bill where sender = '%s' OR receiver = '%s";

LevelDBLocalImpl::LevelDBLocalImpl() {
	leveldb::Status s;
	leveldb::Options options;
	options.create_if_missing = true;
	s = leveldb::DB::Open(options, level_db_name, &_db);
	assert(s.ok());

	//load or set 0 -- index
	std::string value;
	s = _db->Get(leveldb::ReadOptions(), PropertyKey(), &value);
	if (s.IsNotFound()) {
		s = _db->Put(leveldb::WriteOptions(), PropertyKey(), std::to_string(_cur_index.load()));
		assert(s.ok());
	}
	else {
		_cur_index.store(atoi(value.c_str()));
	}

	bool init = true;
	std::ifstream is(sql_db_name);
	if (is.good()) {
		init = false;
	}
	sqlite3_open(sql_db_name, &_sdb);
	
	if (init) {
		//create table
		char* szMsg = NULL;
		sqlite3_exec(_sdb, create_tb_sql, NULL, NULL, &szMsg);
	}
}

void LevelDBLocalImpl::onLocalSync(std::vector<metatradenode::Block>& chain){
	leveldb::Status s;
	std::string value;

	auto iter = chain.begin();
	while (chain.size() > 1) {
		auto& block = *iter;
		auto& trade_list = block.block_body;
		for (auto& trade : trade_list) {
			const std::string& sender = trade.senderAddress;
			const std::string& receiver = trade.receiverAddress;
			std::string item_id;
			long long amount;

			char sql_buf[256];
			if (trade.amount != 0) {
				//cash
				amount = trade.amount;
				item_id = std::to_string(0);
			}
			else {
				//item
				auto ptr = cJSON_Parse(trade.description.c_str());
				item_id = cJSON_GetStringValue(cJSON_GetObjectItem(ptr, "id"));
				amount = cJSON_GetNumberValue(cJSON_GetObjectItem(ptr, "amount"));
				cJSON_Delete(ptr);
			}
			sprintf_s(sql_buf, 256, insert_td_sql, sender.c_str(), receiver.c_str(), item_id.c_str(), amount, trade.timestamp);

			//Sender
			std::string sender_key = Key(sender, item_id);
			s = _db->Get(leveldb::ReadOptions(), sender_key, &value);
			if (s.IsNotFound()) {
				//Put
				if (item_id == "0") {
					s = _db->Put(leveldb::WriteOptions(), sender_key, std::to_string(-amount - trade.commission));
					assert(s.ok());
				}
				else {
					s = _db->Put(leveldb::WriteOptions(), sender_key, std::to_string(-amount));
					assert(s.ok());
				}
			}
			else {
				//Update
				long long v = atoll(value.c_str());

				leveldb::WriteBatch batch;
				batch.Delete(sender_key);

				if (item_id == "0") {
					batch.Put(sender_key, std::to_string(v - amount - trade.commission));
				}
				else {
					batch.Put(sender_key, std::to_string(v - amount));
				}
				s = _db->Write(leveldb::WriteOptions(), &batch);
				assert(s.ok());
			}

			//Receiver
			std::string receiver_key = Key(receiver, item_id);
			s = _db->Get(leveldb::ReadOptions(), receiver_key, &value);
			if (s.IsNotFound()) {
				//Put
				s = _db->Put(leveldb::WriteOptions(), receiver_key, std::to_string(amount));
				assert(s.ok());
			}
			else {
				//Update
				long long v = atoll(value.c_str());

				leveldb::WriteBatch batch;
				batch.Delete(receiver_key);
				batch.Put(receiver_key, std::to_string(v + amount));
				s = _db->Write(leveldb::WriteOptions(), &batch);
				assert(s.ok());
			}

			//insert sqlite
			char* zErrMsg = NULL;
			sqlite3_exec(_sdb, sql_buf, NULL, NULL, &zErrMsg);
			if (zErrMsg)
			{
				sqlite3_free(zErrMsg);
			}
		}

		iter = chain.erase(iter);
		_cur_index.fetch_add(1);
	}
	updateIndexLocal();
}

bool LevelDBLocalImpl::isBalanceTrade(metatradenode::Trade) {
	return true;
}

long long LevelDBLocalImpl::queryAmount(std::string address, std::string item_id) {
	long long amount = 0;
	leveldb::Status s;
	std::string value;
	s = _db->Get(leveldb::ReadOptions(), Key(address, item_id), &value);
	if (!s.IsNotFound()) {
		amount += atol(value.c_str());
	}

	s = _db->Get(leveldb::ReadOptions(), Key(metatradenode::BORADCAST_ADDRESS, item_id), &value);
	if (!s.IsNotFound()) {
		amount += atol(value.c_str());
	}
	
	return amount;
}

void LevelDBLocalImpl::queryBills(std::string address, metatradenode::Bill** bills, uint64_t* sz){
	char sql_buf[256];
	sprintf_s(sql_buf, 256, select_td_sql, address.c_str(), address.c_str());

	int nCol = -1;
	int nRow = -1;
	int index = -1;
	char** azResult = NULL;
	char* errMsg = NULL;

	int result = sqlite3_get_table(_sdb, sql_buf, &azResult, &nRow, &nCol, &errMsg);

	bills = new metatradenode::Bill * [nRow];
	*sz = nRow;

	int idx = nCol;
	for (int i = 0; i < nRow; i++) {
		bills[i] = new metatradenode::Bill();
		strcpy_s(bills[i]->sender, 35, azResult[idx]);
		strcpy_s(bills[i]->receiver, 35, azResult[idx + 1]);
		strcpy_s(bills[i]->id, 10, azResult[idx + 2]);
		bills[i]->amount = atoll(azResult[idx + 3]);
		bills[i]->commission = atoll(azResult[idx + 4]);
		bills[i]->timestamp = atoll(azResult[idx + 5]);

		idx += nCol;
	}
	
	if (azResult) {
		sqlite3_free_table(azResult);
	}
	if (errMsg) {
		sqlite3_free(errMsg);
	}
}

void LevelDBLocalImpl::updateIndexLocal(){
	//update index

	leveldb::Status s;
	leveldb::WriteBatch batch;
	batch.Delete(PropertyKey());
	batch.Put(PropertyKey(), std::to_string(_cur_index));
	s = _db->Write(leveldb::WriteOptions(), &batch);
	assert(s.ok());
}
