#include "MetaTradeNode.h"
#include <chrono>

void metatradenode::MetaTradeNode::init(){
    _client = new metatradenode::MetaTradeClient();
    _bc_service = new MetaTradeBlockchainImpl(_config.address);
    _lc_service = new LevelDBLocalImpl();

    _client->RegisterService(_bc_service, _lc_service);
    _bc_service->RegisterClient(_client);
    _bc_service->RegisterLocal(_lc_service);
}

void metatradenode::MetaTradeNode::run(bool sync){
    if(_config.mining){
        //mining thread
        this->_bc_service->Mining();
    }

    if (sync) {
        _client->RunSync();
    }
    else {
        _client->RunAsync();
    }
}

void metatradenode::MetaTradeNode::reload(){
    this->_bc_service->SendSyncRequest();
}

long long metatradenode::MetaTradeNode::queryAmount(const char* address, const char* item_id) {
    if (_lc_service != nullptr) {
        long long prev = _lc_service->queryAmount(address, item_id);
        long long cur = _bc_service->queryAmount(address, item_id);
        return prev + cur;
    }
    else {
        return 0;
    }
}

long long metatradenode::MetaTradeNode::queryTransitAmount(const char* address, const char* item_id){
    if (_bc_service != nullptr) {
        long long cur = _bc_service->queryTransitAmount(address, item_id);
        return cur;
    }
    else {
        return 0;
    }
}

void metatradenode::MetaTradeNode::submitTrade(const char* receiver, const char* item_id, long long amount) {
    std::string item = item_id;
    metatradenode::Trade trade;
    trade.senderAddress = _config.address;
    trade.receiverAddress = receiver;
    trade.senderPublicKey = _config.pubkey;
    trade.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    if (item == "0") {
        //cash
        trade.amount = amount;
        trade.commission = amount / metatradenode::commission_rate;
        trade.description = "";
    }
    else {
        //item
        auto ptr = cJSON_CreateObject();
        cJSON_AddItemToObject(ptr, "id", cJSON_CreateString(item_id));
        cJSON_AddItemToObject(ptr, "amount", cJSON_CreateNumber(amount));
        trade.amount = 0;
        trade.commission = metatradenode::commission_item_fix;
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

        //wait client stop and delete
        delete _client;

        //wait mining thread and delete
        delete _bc_service;

        //delete service
        delete _lc_service;
    }
}
