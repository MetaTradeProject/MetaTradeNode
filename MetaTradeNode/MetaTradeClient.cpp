#include "MetaTradeClient.h"
#include "BlockChainService.h"
#include "LocalService.h"

void metatradenode::MetaTradeClient::OnConnected() {
	std::string sub_str = std::string(SUB_INIT).append("/").append(std::to_string(_local_service->getStartIndex()));
	this->Subscribe(sub_str.c_str(), std::bind(&BlockchainService::Init, _bc_service, std::placeholders::_1));
	_status = Status::SEND_INIT;
}

void metatradenode::MetaTradeClient::OnDisconnected() {
	_bc_service->Stop();
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
	this->Subscribe(SUB_TRADE, std::bind(&BlockchainService::onTrade, _bc_service, std::placeholders::_1));
	this->Subscribe(SUB_SPAWN, std::bind(&BlockchainService::onSpawn, _bc_service, std::placeholders::_1));
	this->Subscribe(SUB_JUDGE, std::bind(&BlockchainService::onJudge, _bc_service, std::placeholders::_1));
	this->Subscribe(SUB_SEMI_SYNC, std::bind(&BlockchainService::onSemiSync, _bc_service, std::placeholders::_1));
	this->Subscribe(SUB_SYNC, std::bind(&BlockchainService::onSync, _bc_service, std::placeholders::_1));
	_status = Status::SUB_ALL;
	_status = Status::FINISHED;
}

metatradenode::MetaTradeClient::~MetaTradeClient() {
	if (_async_thread != nullptr) {
		_async_thread->join();
		delete _async_thread;
	}
}
