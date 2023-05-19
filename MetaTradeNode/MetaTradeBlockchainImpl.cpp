#include "MetaTradeBlockchainImpl.h"
#include "LocalService.h"
#include "CryptoUtils.h"
#include <cJSON/cJSON.h>
#include <chrono>

void MetaTradeBlockchainImpl::Init(webstomppp::StompCallbackMsg msg) {
	std::unique_lock<std::mutex> ul(_lock);
	ParseSyncMessage(msg.body);
	if(this->_rawblock_deque.size() != 0){
		this->_proof_done.store(false);
		_cond.notify_one();
	}
	ul.unlock();
	_local->onLocalSync(this->_chain);
	BlockchainService::Init(msg);
}

MetaTradeBlockchainImpl::~MetaTradeBlockchainImpl(){
	if (_mining_thread != nullptr) {
		_mining_thread->join();
		delete _mining_thread;
	}
}

void MetaTradeBlockchainImpl::onSpawn(webstomppp::StompCallbackMsg msg) {
	cJSON* root = cJSON_Parse(msg.body);
	int proofLevel = cJSON_GetObjectItem(root, "proofLevel")->valueint;
	cJSON_Delete(root);

	metatradenode::RawBlock raw_block;
	raw_block.proof_level = proofLevel;
	raw_block.block_body.insert(raw_block.block_body.end(), this->_trade_list.begin(), this->_trade_list.end());
	this->_trade_list.erase(this->_trade_list.begin(), this->_trade_list.end());
	
	{
		std::unique_lock<std::mutex> ul(_lock);
		this->_rawblock_deque.push_back(raw_block);
		if(this->_rawblock_deque.size() != 0){
			this->_proof_done.store(false);
			_cond.notify_one();
		}
	}
}

void MetaTradeBlockchainImpl::onSemiSync(webstomppp::StompCallbackMsg msg){
	std::unique_lock<std::mutex> ul(_lock);
	metatradenode::Block block;
	ParseSemiSyncMessage(msg.body, block);
	if(this->_rawblock_deque.size() != 0){
		this->_proof_done.store(false);
		_cond.notify_one();
	}
	ul.unlock();
	this->_chain.emplace_back(block);
	_local->onLocalSync(this->_chain);
}

void MetaTradeBlockchainImpl::onSync(webstomppp::StompCallbackMsg msg) {
	std::unique_lock<std::mutex> ul(_lock);
	ParseSyncMessage(msg.body);
	if(this->_rawblock_deque.size() != 0){
		this->_proof_done.store(false);
		_cond.notify_one();
	}
	ul.unlock();
	_local->onLocalSync(this->_chain);
}

void MetaTradeBlockchainImpl::onTrade(webstomppp::StompCallbackMsg msg) {
	cJSON* root = cJSON_Parse(msg.body);
	metatradenode::Trade trade;
	ParseTrade(root, trade);
	this->_trade_list.emplace_back(trade);
	cJSON_Delete(root);
}

void MetaTradeBlockchainImpl::onJudge(webstomppp::StompCallbackMsg msg){
	std::unique_lock<std::mutex> sl(_lock);
	cJSON* root = cJSON_Parse(msg.body);
	int proof = cJSON_GetObjectItem(root, "proof")->valueint;

	if (isValidProof(proof, this->_rawblock_deque.front())) {
		SendAgreeMessage(proof);
	}
}

void MetaTradeBlockchainImpl::Stop() {
	//stop mining
	this->_quit_flag.store(true);
}

bool MetaTradeBlockchainImpl::isValidTrade(metatradenode::Trade& trade) {
	return CryptoUtils::isValidAddress(trade.senderAddress.c_str())
		&& (trade.receiverAddress == metatradenode::BORADCAST_ADDRESS ? true: CryptoUtils::isValidAddress(trade.receiverAddress.c_str()))
		&& CryptoUtils::isValidSignature(trade.getHash().c_str(), trade.signature.c_str(), trade.senderPublicKey.c_str())
		&& _local->isBalanceTrade(trade);
}

bool MetaTradeBlockchainImpl::isValidProof(int proof, metatradenode::RawBlock& raw_block) {
	metatradenode::Block block;
	for (auto& trade : raw_block.block_body) {
		if (isValidTrade(trade)) {
			block.block_body.push_back(trade);
		}
	}

	block.prev_hash = this->_chain.back().getHash();
	block.calMerkleHash();
	block.proof_level = raw_block.proof_level;
	block.proof = proof;

	std::stringstream ss;
	ss << block.prev_hash << block.merkle_hash << block.proof_level << block.proof;
	if (CryptoUtils::GetSha256(CryptoUtils::GetSha256(ss.str().c_str()).c_str()).substr(0, block.proof_level) == std::string(block.proof_level, '0')) {
		return true;
	}
	else {
		return false;
	}
}

void MetaTradeBlockchainImpl::Mining(){
	this->_mining_thread = new std::thread(&MetaTradeBlockchainImpl::MiningBlock, this);
}

void MetaTradeBlockchainImpl::MiningBlock(){
	do{
		std::unique_lock<std::mutex> ul(_lock);
		_cond.wait(ul, [this]()->bool{
			if (this->_quit_flag.load()){
				return true;
			}
			else{
				return !(this->_rawblock_deque.empty() || this->_proof_done.load());
			}
		});
		//quit
		if(this->_quit_flag.load()){
			break;
		}

		auto& raw_block = this->_rawblock_deque.front();
		ul.unlock();

		metatradenode::Block block;
		for (auto& trade : raw_block.block_body) {
			if (isValidTrade(trade)) {
				block.block_body.push_back(trade);
			}
		}

		block.prev_hash = this->_chain.back().getHash();
		block.calMerkleHash();
		block.proof_level = raw_block.proof_level;

		std::string mine_data = block.prev_hash + block.merkle_hash + std::to_string(block.proof_level);
		int proof = 0;
		int proof_level = block.proof_level;

		std::string target(proof_level, '0');
		while (true) {
			std::string guess = mine_data + std::to_string(proof);
			if (CryptoUtils::GetSha256(CryptoUtils::GetSha256(guess.c_str()).c_str()).substr(0, proof_level) == target) {
				break;
			}
			else {
				proof++;
			}
		}
		block.proof = proof;
		SendProofMessage(block);
		_proof_done.store(true);
	}
	while(true);
}

void MetaTradeBlockchainImpl::SendAgreeMessage(int proof) {
	cJSON* agr_msg = cJSON_CreateObject();
	cJSON_AddItemToObject(agr_msg, "address", cJSON_CreateString(this->_wallet_address.c_str()));
	cJSON_AddItemToObject(agr_msg, "proof", cJSON_CreateNumber(proof));
	
	_client->SendJson(metatradenode::POST_AGREE, cJSON_PrintUnformatted(agr_msg));
	cJSON_Delete(agr_msg);
}

void MetaTradeBlockchainImpl::SendSyncRequest(){
	cJSON* loc_msg = cJSON_CreateObject();
	cJSON_AddItemToObject(loc_msg, "startIndex", cJSON_CreateNumber(_local->getStartIndex()));

	_client->SendJson(metatradenode::POST_SYNC, cJSON_PrintUnformatted(loc_msg));
	cJSON_Delete(loc_msg);
}

long long MetaTradeBlockchainImpl::queryAmount(std::string address, std::string item_id) {
	std::unique_lock<std::mutex> sl(_lock);
	std::vector<metatradenode::Block> snapshot(this->_chain);
	sl.unlock();

	bool isCash = true;
	if (item_id != "0") {
		isCash = false;
	}

	long long cur = 0;
	for (auto& block : snapshot) {
		auto& trade_list = block.block_body;

		for (auto& trade : trade_list) {
			const std::string& sender = trade.senderAddress;
			const std::string& receiver = trade.receiverAddress;

			if (sender == address) {
				if (isCash) {
					cur = cur - trade.amount - trade.commission;
				}
				else {
					//item
					auto ptr = cJSON_Parse(trade.description.c_str());
					std::string this_item_id = cJSON_GetStringValue(cJSON_GetObjectItem(ptr, "id"));
					long long amount = cJSON_GetNumberValue(cJSON_GetObjectItem(ptr, "amount"));
					cJSON_Delete(ptr);

					if (this_item_id == item_id) {
						cur -= amount;
					}
				}
			}
			
			if (receiver == address) {
				if (isCash) {
					cur = cur + trade.amount;
				}
				else {
					//item
					auto ptr = cJSON_Parse(trade.description.c_str());
					std::string this_item_id = cJSON_GetStringValue(cJSON_GetObjectItem(ptr, "id"));
					long long amount = cJSON_GetNumberValue(cJSON_GetObjectItem(ptr, "amount"));
					cJSON_Delete(ptr);

					if (this_item_id == item_id) {
						cur += amount;
					}
				}
			}
			
		}
	}
	return cur;
}

long long MetaTradeBlockchainImpl::queryTransitAmount(std::string address, std::string item_id) {
	std::unique_lock<std::mutex> sl(_lock);
	auto rb_snapshot(this->_rawblock_deque);
	auto td_snapshot(this->_trade_list);
	sl.unlock();

	bool isCash = true;
	if (item_id != "0") {
		isCash = false;
	}

	long long cur = 0;
	for (auto& raw_block : rb_snapshot) {
		auto& trade_list = raw_block.block_body;

		for (auto& trade : trade_list) {
			const std::string& sender = trade.senderAddress;
			const std::string& receiver = trade.receiverAddress;

			if (sender == address) {
				if (isCash) {
					cur = cur - trade.amount - trade.commission;
				}
				else {
					//item
					auto ptr = cJSON_Parse(trade.description.c_str());
					std::string this_item_id = cJSON_GetStringValue(cJSON_GetObjectItem(ptr, "id"));
					long long amount = cJSON_GetNumberValue(cJSON_GetObjectItem(ptr, "amount"));
					cJSON_Delete(ptr);

					if (this_item_id == item_id) {
						cur -= amount;
					}
				}
			}
			
			if (receiver == address) {
				if (isCash) {
					cur = cur + trade.amount;
				}
				else {
					//item
					auto ptr = cJSON_Parse(trade.description.c_str());
					std::string this_item_id = cJSON_GetStringValue(cJSON_GetObjectItem(ptr, "id"));
					long long amount = cJSON_GetNumberValue(cJSON_GetObjectItem(ptr, "amount"));
					cJSON_Delete(ptr);

					if (this_item_id == item_id) {
						cur += amount;
					}
				}
			}
			
		}
	}

	for (auto& trade : td_snapshot) {
		const std::string& sender = trade.senderAddress;
		const std::string& receiver = trade.receiverAddress;

		if (sender == address) {
			if (isCash) {
				cur = cur - trade.amount - trade.commission;
			}
			else {
				//item
				auto ptr = cJSON_Parse(trade.description.c_str());
				std::string this_item_id = cJSON_GetStringValue(cJSON_GetObjectItem(ptr, "id"));
				long long amount = cJSON_GetNumberValue(cJSON_GetObjectItem(ptr, "amount"));
				cJSON_Delete(ptr);

				if (this_item_id == item_id) {
					cur -= amount;
				}
			}
		}
		
		if (receiver == address) {
			if (isCash) {
				cur = cur + trade.amount;
			}
			else {
				//item
				auto ptr = cJSON_Parse(trade.description.c_str());
				std::string this_item_id = cJSON_GetStringValue(cJSON_GetObjectItem(ptr, "id"));
				long long amount = cJSON_GetNumberValue(cJSON_GetObjectItem(ptr, "amount"));
				cJSON_Delete(ptr);

				if (this_item_id == item_id) {
					cur += amount;
				}
			}
		}
		
	}

	return cur;
}

void MetaTradeBlockchainImpl::SendTrade(metatradenode::Trade& trade) {
	cJSON* trade_msg = cJSON_CreateObject();

	cJSON_AddItemToObject(trade_msg, "senderAddress", cJSON_CreateString(trade.senderAddress.c_str()));
	cJSON_AddItemToObject(trade_msg, "receiverAddress", cJSON_CreateString(trade.receiverAddress.c_str()));
	cJSON_AddItemToObject(trade_msg, "amount", cJSON_CreateNumber(trade.amount));
	cJSON_AddItemToObject(trade_msg, "commission", cJSON_CreateNumber(trade.commission));
	cJSON_AddItemToObject(trade_msg, "timestamp", cJSON_CreateNumber(trade.timestamp));
	cJSON_AddItemToObject(trade_msg, "signature", cJSON_CreateString(trade.signature.c_str()));
	cJSON_AddItemToObject(trade_msg, "senderPublicKey", cJSON_CreateString(trade.senderPublicKey.c_str()));
	cJSON_AddItemToObject(trade_msg, "description", cJSON_CreateString(trade.description.c_str()));


	_client->SendJson(metatradenode::POST_TRADE, cJSON_PrintUnformatted(trade_msg));
	cJSON_Delete(trade_msg);
}

void MetaTradeBlockchainImpl::SendProofMessage(metatradenode::Block& block) {
	cJSON* proof_msg = cJSON_CreateObject();
	cJSON* block_msg = cJSON_CreateObject();
	cJSON* body = cJSON_CreateArray();

	for (auto& trade : block.block_body) {
		cJSON* trade_msg = cJSON_CreateObject();

		cJSON_AddItemToObject(trade_msg, "senderAddress", cJSON_CreateString(trade.senderAddress.c_str()));
		cJSON_AddItemToObject(trade_msg, "receiverAddress", cJSON_CreateString(trade.receiverAddress.c_str()));
		cJSON_AddItemToObject(trade_msg, "amount", cJSON_CreateNumber(trade.amount));
		cJSON_AddItemToObject(trade_msg, "commission", cJSON_CreateNumber(trade.commission));
		cJSON_AddItemToObject(trade_msg, "timestamp", cJSON_CreateNumber(trade.timestamp));
		cJSON_AddItemToObject(trade_msg, "signature", cJSON_CreateString(trade.signature.c_str()));
		cJSON_AddItemToObject(trade_msg, "senderPublicKey", cJSON_CreateString(trade.senderPublicKey.c_str()));
		cJSON_AddItemToObject(trade_msg, "description", cJSON_CreateString(trade.description.c_str()));
		cJSON_AddItemToArray(body, trade_msg);
	}
	cJSON_AddItemToObject(block_msg, "prevHash", cJSON_CreateString(block.prev_hash.c_str()));
	cJSON_AddItemToObject(block_msg, "merkleHash", cJSON_CreateString(block.merkle_hash.c_str()));
	cJSON_AddItemToObject(block_msg, "proofLevel", cJSON_CreateNumber(block.proof_level));
	cJSON_AddItemToObject(block_msg, "proof", cJSON_CreateNumber(block.proof));
	cJSON_AddItemToObject(block_msg, "blockBody", body);

	cJSON_AddItemToObject(proof_msg, "address", cJSON_CreateString(_wallet_address.c_str()));
	cJSON_AddItemToObject(proof_msg, "block", block_msg);

	_client->SendJson(metatradenode::POST_PROOF, cJSON_PrintUnformatted(proof_msg));
	cJSON_Delete(proof_msg);
}

void MetaTradeBlockchainImpl::ParseSyncMessage(const char* raw) {
	cJSON* root = cJSON_Parse(raw);
	cJSON* chain_root = cJSON_GetObjectItem(root, "chain");
	cJSON* raw_blocks_root = cJSON_GetObjectItem(root, "rawBlocks");
	cJSON* trades_root = cJSON_GetObjectItem(root, "tradeList");

	this->_chain.clear();
	ParseChain(chain_root, this->_chain);
	this->_rawblock_deque.clear();
	ParseRawBlocks(raw_blocks_root, this->_rawblock_deque);
	this->_trade_list.clear();
	ParseTradeList(trades_root, this->_trade_list);
	cJSON_Delete(root);
}

void MetaTradeBlockchainImpl::ParseSemiSyncMessage(const char* raw, metatradenode::Block& block) {
	cJSON* root = cJSON_Parse(raw);
	cJSON* block_root = cJSON_GetObjectItem(root, "block");
	cJSON* raw_blocks_root = cJSON_GetObjectItem(root, "rawBlocks");
	cJSON* trades_root = cJSON_GetObjectItem(root, "tradeList");

	ParseBlock(block_root, block);
	this->_rawblock_deque.clear();
	ParseRawBlocks(raw_blocks_root, this->_rawblock_deque);
	this->_trade_list.clear();
	ParseTradeList(trades_root, this->_trade_list);
	cJSON_Delete(root);
}

void MetaTradeBlockchainImpl::ParseChain(cJSON* root, std::vector<metatradenode::Block>& chain){
	int sz = cJSON_GetArraySize(root);

	for (int i = 0; i < sz; i++) {
		cJSON* td = cJSON_GetArrayItem(root, i);
		metatradenode::Block block;
		ParseBlock(td, block);
		chain.push_back(block);
	}
}

void MetaTradeBlockchainImpl::ParseRawBlocks(cJSON* root, std::deque<metatradenode::RawBlock>& raw_blocks) {
	int sz = cJSON_GetArraySize(root);

	for (int i = 0; i < sz; i++) {
		cJSON* td = cJSON_GetArrayItem(root, i);
		metatradenode::RawBlock raw_block;
		ParseRawBlock(td, raw_block);
		raw_blocks.push_back(raw_block);
	}
}

void MetaTradeBlockchainImpl::ParseTradeList(cJSON* root, std::vector<metatradenode::Trade>& vec){
	int sz = cJSON_GetArraySize(root);

	for (int i = 0; i < sz; i++) {
		cJSON* td = cJSON_GetArrayItem(root, i);
		metatradenode::Trade trade;
		ParseTrade(td, trade);
		vec.emplace_back(trade);
	}
}

void MetaTradeBlockchainImpl::ParseTrade(cJSON* root, metatradenode::Trade& trade) {
	trade.senderAddress = cJSON_GetStringValue(cJSON_GetObjectItem(root, "senderAddress"));
	trade.receiverAddress = cJSON_GetStringValue(cJSON_GetObjectItem(root, "receiverAddress"));
	trade.amount = cJSON_GetNumberValue(cJSON_GetObjectItem(root, "amount"));
	trade.commission = cJSON_GetNumberValue(cJSON_GetObjectItem(root, "commission"));
	trade.timestamp = cJSON_GetNumberValue(cJSON_GetObjectItem(root, "timestamp"));
	trade.signature = cJSON_GetStringValue(cJSON_GetObjectItem(root, "signature"));
	trade.senderPublicKey = cJSON_GetStringValue(cJSON_GetObjectItem(root, "senderPublicKey"));
	trade.description = cJSON_GetStringValue(cJSON_GetObjectItem(root, "description"));
}

void MetaTradeBlockchainImpl::ParseRawBlock(cJSON* root, metatradenode::RawBlock& raw_block) {
	raw_block.proof_level = cJSON_GetObjectItem(root, "proofLevel")->valueint;
	ParseTradeList(cJSON_GetObjectItem(root, "blockBody"), raw_block.block_body);
}

void MetaTradeBlockchainImpl::ParseBlock(cJSON* root, metatradenode::Block& block){
	block.prev_hash = cJSON_GetStringValue(cJSON_GetObjectItem(root, "prevHash"));
	block.merkle_hash = cJSON_GetStringValue(cJSON_GetObjectItem(root, "merkleHash"));
	block.proof_level = cJSON_GetObjectItem(root, "proofLevel")->valueint;
	block.proof = cJSON_GetObjectItem(root, "proof")->valueint;
	ParseTradeList(cJSON_GetObjectItem(root, "blockBody"), block.block_body);
}
