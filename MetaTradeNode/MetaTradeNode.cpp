#include "MetaTradeNode.h"
#include <chrono>
#include <thread>

void metatradenode::MetaTradeNode::init(){
    _client = new metatradenode::MetaTradeClient();
    _bc_service = new MetaTradeBlockchainImpl();
    _lc_service = new LevelDBLocalImpl();

    _client->RegisterService(_bc_service, _lc_service);
    _bc_service->RegisterClient(_client);
    _bc_service->RegisterLocal(_lc_service);
}

void metatradenode::MetaTradeNode::run(bool sync){
    //mining thread
    this->_mining_thread = new std::thread(&MetaTradeBlockchainImpl::MiningBlock, this->_bc_service);
    this->_mining_thread

    if (sync) {
        _client->RunSync();
    }
    else {
        _client->RunAsync();
    }
}

long metatradenode::MetaTradeNode::queryAmount(const char* address, const char* item_id) {
    if (_lc_service != nullptr) {
        return _lc_service->queryAmount(address, item_id);
    }
    else {
        return 0;
    }
}

void metatradenode::MetaTradeNode::submitTrade(const char* receiver, const char* item_id, long amount) {
    std::string item = item_id;
    metatradenode::Trade trade;
    trade.senderAddress = _config.address;
    trade.receiverAddress = receiver;
    trade.commission = 500;
    trade.senderPublicKey = _config.pubkey;
    trade.timestamp = std::chrono::duration_cast<milliseconds>(std::chrono::system_clock::now().time_since_epoch());

    if (item == "0") {
        //cash
        trade.amount = amount;
        trade.description = "";
    }
    else {
        //item
        auto ptr = cJSON_CreateObject();
        cJSON_AddItemToObject(ptr, "id", cJSON_CreateString(item_id));
        cJSON_AddItemToObject(ptr, "amount", cJSON_CreateNumber(amount));
        trade.amount = 0;
        trade.description = cJSON_PrintUnformatted(ptr);
        cJSON_Delete(ptr);
    }
    
    char* signature;
    CryptoUtils::SignTrade(trade.getHash().c_str(), _config.prikey, signature);
    trade.signature = signature;
    _bc_service->SendTrade(trade);
}

metatradenode::MetaTradeNode::~MetaTradeNode(){
    if (_client != nullptr) {
        //send disconnect
        _client->Disconnect();

        //wait mining thread
        _mining_thread->join();
        delete _mining_thread;

        //wait client stop and delete
        delete _client;

        //delete service
        delete _lc_service;
        delete _bc_service;
    }
}
