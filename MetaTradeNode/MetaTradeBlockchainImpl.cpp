#include "MetaTradeBlockchainImpl.h"
#include "LocalService.h"
#include "CryptoUtils.h"
#include <cJSON/cJSON.h>

void MetaTradeBlockchainImpl::Init(webstomppp::StompCallbackMsg msg) {
	ParseSyncMessage(msg.body);
	_local->onLocalSync(msg.body);
	BlockchainService::Init(msg);
}

void MetaTradeBlockchainImpl::onTrade(webstomppp::StompCallbackMsg msg) {
	cJSON* root = cJSON_Parse(msg.body);
	metatradenode::Trade trade;
	ParseTrade(root, trade);
	this->_trade_list.emplace_back(trade);
	cJSON_Delete(root);
}

void MetaTradeBlockchainImpl::onSpawn(webstomppp::StompCallbackMsg msg) {
	cJSON* root = cJSON_Parse(msg.body);
	int proofLevel = cJSON_GetObjectItem(root, "proofLevel")->valueint;

	metatradenode::RawBlock raw_block;
	raw_block.proof_level = proofLevel;
	raw_block.block_body.insert(raw_block.block_body.end(), this->_trade_list.begin(), this->_trade_list.end());
	this->_trade_list.erase(this->_trade_list.begin(), this->_trade_list.end());
	this->_rawblock_deque.push_back(raw_block);
	cJSON_Delete(root);
}

void MetaTradeBlockchainImpl::onJudge(webstomppp::StompCallbackMsg msg){
	cJSON* root = cJSON_Parse(msg.body);
	int proof = cJSON_GetObjectItem(root, "proofLevel")->valueint;

	if (isValidProof(proof, this->_rawblock_deque.front())) {
		SendAgreeMessage(proof);
	}
}

void MetaTradeBlockchainImpl::onSemiSync(webstomppp::StompCallbackMsg msg)
{
	metatradenode::Block block;
	ParseSemiSyncMessage(msg.body, block);

	this->_chain.emplace_back(block);
	_local->onSemiSync(msg.body);
}

void MetaTradeBlockchainImpl::onSync(webstomppp::StompCallbackMsg msg) {
	ParseSyncMessage(msg.body);
	_local->onLocalSync(msg.body);
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

void MetaTradeBlockchainImpl::Stop() {

}

void MetaTradeBlockchainImpl::ParseChain(cJSON* root, std::vector<metatradenode::Block>& chain){
	int sz = cJSON_GetArraySize(root);

	for (int i = 0; i < sz; i++) {
		cJSON* td = cJSON_GetArrayItem(root, i);
		metatradenode::Block block;
		ParseBlock(root, block);
		chain.push_back(block);
	}
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

void MetaTradeBlockchainImpl::ParseRawBlocks(cJSON* root, std::deque<metatradenode::RawBlock>& raw_blocks) {
	int sz = cJSON_GetArraySize(root);

	for (int i = 0; i < sz; i++) {
		cJSON* td = cJSON_GetArrayItem(root, i);
		metatradenode::RawBlock raw_block;
		ParseRawBlock(root, raw_block);
		raw_blocks.push_back(raw_block);
	}
}

void MetaTradeBlockchainImpl::ParseTradeList(cJSON* root, std::vector<metatradenode::Trade>& vec){
	int sz = cJSON_GetArraySize(root);

	for (int i = 0; i < sz; i++) {
		cJSON* td = cJSON_GetArrayItem(root, i);
		metatradenode::Trade trade;
		ParseTrade(root, trade);
		vec.emplace_back(trade);
	}
}

void MetaTradeBlockchainImpl::ParseTrade(cJSON* root, metatradenode::Trade& trade) {
	trade.senderAddress = cJSON_GetStringValue(cJSON_GetObjectItem(root, "senderAddress"));
	trade.receiverAddress = cJSON_GetStringValue(cJSON_GetObjectItem(root, "receiverAddress"));
	trade.amount = cJSON_GetNumberValue(cJSON_GetObjectItem(root, "amount"));
	trade.commission = cJSON_GetNumberValue(cJSON_GetObjectItem(root, "commission"));
	trade.timestamp = strtoll(cJSON_GetStringValue(cJSON_GetObjectItem(root, "timestamp")), nullptr, 10);
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

bool MetaTradeBlockchainImpl::isValidTrade(metatradenode::Trade& trade) {
	return CryptoUtils::isValidAddress(trade.senderAddress.c_str())
		&& CryptoUtils::isValidAddress(trade.receiverAddress.c_str())
		&& CryptoUtils::isValidSignature(trade.getHash().c_str(), trade.signature.c_str(), trade.senderPublicKey.c_str())
		&& _local->isBalanceTrade(trade);
}

void MetaTradeBlockchainImpl::MiningBlock(){
	auto& raw_block = this->_rawblock_deque.front();

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
		std::string guess = mine_data.append(std::to_string(proof));
		if (CryptoUtils::GetSha256(CryptoUtils::GetSha256(guess.c_str()).c_str()).substr(0, proof_level) == target) {
			break;
		}
		else {
			proof++;
		}
	}
	block.proof = proof;
	SendProofMessage(block);
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
	if (CryptoUtils::GetSha256(CryptoUtils::GetSha256(ss.str().c_str()).c_str()) == std::string(block.proof_level, '0')) {
		return true;
	}
	else {
		return false;
	}
}
void MetaTradeBlockchainImpl::SendAgreeMessage(int proof) {
	cJSON* agr_msg = cJSON_CreateObject();
	cJSON_AddItemToObject(agr_msg, "address", cJSON_CreateString(this->_wallet_address.c_str()));
	cJSON_AddItemToObject(agr_msg, "proof", cJSON_CreateNumber(proof));
	_client->Send(cJSON_PrintUnformatted(agr_msg));
	cJSON_Delete(agr_msg);
}

void MetaTradeBlockchainImpl::SendSyncRequest(){
	cJSON* loc_msg = cJSON_CreateObject();
	cJSON_AddItemToObject(loc_msg, "startIndex", cJSON_CreateNumber(_local->getStartIndex()));
	_client->Send(cJSON_PrintUnformatted(loc_msg));
	cJSON_Delete(loc_msg);
}

void MetaTradeBlockchainImpl::SendProofMessage(metatradenode::Block& block) {

}
