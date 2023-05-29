#pragma once
#include <webstomp++/WebStompType.h>
#include "MetaTradeClient.h"
#include "MetaTradeBlockchainModels.h"
#include "MiningPublisher.h"
#pragma comment(lib , "cJSON.lib")

namespace metatradenode {
	class BlockchainService
	{
	public:
		void RegisterClient(metatradenode::MetaTradeClient* client) { this->_client = client; };
		void RegisterLocal(metatradenode::LocalService* local) { this->_local = local; };
		void RegisterPublisher(metatradenode::MiningPublisher* publisher) { this->_publisher = publisher; };
		virtual void SendTrade(metatradenode::Trade& trade) = 0;
		virtual void Mining() = 0;
		virtual void SendSyncRequest() = 0;
		virtual long long queryAmount(std::string address, std::string item_id) = 0;
		virtual long long queryTransitAmount(std::string address, std::string item_id) = 0;
		virtual void queryBills(std::string address, std::vector<metatradenode::Bill>& bills) = 0;
		virtual void queryTransitBills(std::string address, std::vector<metatradenode::Bill>& bills) = 0;
		virtual ~BlockchainService() {};
		friend class metatradenode::MetaTradeClient;
	protected:
		virtual void Init(webstomppp::StompCallbackMsg msg) { _client->RegisterSubscribe(); };
		virtual void onTrade(webstomppp::StompCallbackMsg msg) = 0;
		virtual void onSpawn(webstomppp::StompCallbackMsg msg) = 0;
		virtual void onJudge(webstomppp::StompCallbackMsg msg) = 0;
		virtual void onSemiSync(webstomppp::StompCallbackMsg msg) = 0;
		virtual void onSync(webstomppp::StompCallbackMsg msg) = 0;
		virtual void Stop() = 0;
		metatradenode::MetaTradeClient* _client;
		metatradenode::LocalService* _local;
		metatradenode::MiningPublisher* _publisher;
		BlockchainService() :_client(nullptr), _local(nullptr), _publisher(nullptr) {};
	};
}