#include "LevelDBLocalImpl.h"

constexpr const char* PropertyKey() { return "start-index"; };

LevelDBLocalImpl::LevelDBLocalImpl() {
	leveldb::Status s;
	leveldb::Options options;
	options.create_if_missing = true;
	s = leveldb::DB::Open(options, "./test.db", &_db);
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
}

void LevelDBLocalImpl::onLocalSync(std::vector<metatradenode::Block>& chain){
	leveldb::Status s;
	std::string value;

	for (auto& block : chain) {
		auto& trade_list = block.block_body;
		for (auto& trade : trade_list) {
			const std::string& sender = trade.senderAddress;
			const std::string& receiver = trade.receiverAddress;
			std::string item_id;
			long amount;

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

			//Sender
			std::string sender_key = Key(sender, item_id);
			s = _db->Get(leveldb::ReadOptions(), sender_key, &value);
			if (s.IsNotFound()) {
				//Put
				s = _db->Put(leveldb::WriteOptions(), sender_key, std::to_string(-amount));
				assert(s.ok());
			}
			else {
				//Update
				long v = atol(value.c_str());

				leveldb::WriteBatch batch;
				batch.Delete(sender_key);
				batch.Put(sender_key, std::to_string(v - amount));
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
				long v = atol(value.c_str());

				leveldb::WriteBatch batch;
				batch.Delete(receiver_key);
				batch.Put(receiver_key, std::to_string(v + amount));
				s = _db->Write(leveldb::WriteOptions(), &batch);
				assert(s.ok());
			}
		}
		_cur_index.fetch_add(1);
	}
}

void LevelDBLocalImpl::onSemiSync(const metatradenode::Block& block) {
	leveldb::Status s;
	std::string value;
	auto& trade_list = block.block_body;
	for (auto& trade : trade_list) {
		const std::string& sender = trade.senderAddress;
		const std::string& receiver = trade.receiverAddress;
		std::string item_id;
		long amount;

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

		//Sender
		std::string sender_key = Key(sender, item_id);
		s = _db->Get(leveldb::ReadOptions(), sender_key, &value);
		if (s.IsNotFound()) {
			//Put
			s = _db->Put(leveldb::WriteOptions(), sender_key, std::to_string(-amount));
			assert(s.ok());
		}
		else {
			//Update
			long v = atol(value.c_str());

			leveldb::WriteBatch batch;
			batch.Delete(sender_key);
			batch.Put(sender_key, std::to_string(v - amount));
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
			long v = atol(value.c_str());

			leveldb::WriteBatch batch;
			batch.Delete(receiver_key);
			batch.Put(receiver_key, std::to_string(v + amount));
			s = _db->Write(leveldb::WriteOptions(), &batch);
			assert(s.ok());
		}
	}
	_cur_index.fetch_add(1);
}

bool LevelDBLocalImpl::isBalanceTrade(metatradenode::Trade) {
	return true;
}

long LevelDBLocalImpl::queryAmount(std::string address, std::string item_id) {
	leveldb::Status s;
	std::string value;
	s = _db->Get(leveldb::ReadOptions(), Key(address, item_id), &value);
	if (s.IsNotFound()) {
		return 0;
	}
	else {
		return atol(value.c_str());
	}
}