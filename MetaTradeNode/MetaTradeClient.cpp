#include "MetaTradeClient.h"
#include "BlockChainService.h"

void metatradenode::MetaTradeClient::OnConnected() {
	this->Subscribe(SUB_INIT, std::bind(&BlockChainService::Init, _service, std::placeholders::_1));
	_status = Status::SEND_INIT;
}

void metatradenode::MetaTradeClient::RunSync() {
	Connect(metatradenode::STOMP_PATH);
	this->Run();
}

void metatradenode::MetaTradeClient::RunAsync() {
	_async_thread = new std::thread(&metatradenode::MetaTradeClient::RunSync, this);
}

void metatradenode::MetaTradeClient::RegisterSubscribe() {
	_status = Status::SERVICE_INIT;
	this->Subscribe(SUB_TRADE, std::bind(&BlockChainService::onTrade, _service, std::placeholders::_1));
	this->Subscribe(SUB_SPAWN, std::bind(&BlockChainService::onSpawn, _service, std::placeholders::_1));
	this->Subscribe(SUB_JUDGE, std::bind(&BlockChainService::onJudge, _service, std::placeholders::_1));
	this->Subscribe(SUB_SEMI_SYNC, std::bind(&BlockChainService::onSemiSync, _service, std::placeholders::_1));
	this->Subscribe(SUB_SYNC, std::bind(&BlockChainService::onSync, _service, std::placeholders::_1));
	_status = Status::SUB_ALL;
	_status = Status::FINISHED;
}

metatradenode::MetaTradeClient::~MetaTradeClient() {
	if (_async_thread != nullptr) {
		_async_thread->join();
		delete _async_thread;
	}
}
