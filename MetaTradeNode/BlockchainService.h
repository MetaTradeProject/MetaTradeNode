#pragma once
#include <webstomp++/WebStompType.h>
#include "MetaTradeClient.h"
#include "MetaTradeBlockchainModels.h"
#pragma comment(lib , "cJSON.lib")

namespace metatradenode {
	class BlockchainService
	{
	public:
		void RegisterClient(metatradenode::MetaTradeClient* client) { this->_client = client; };
		void RegisterLocal(metatradenode::LocalService* local) { this->_local = local; };
		void SendTrade(metatradenode::Trade& trade) = 0;
		virtual ~BlockchainService() = 0;
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
		BlockchainService() :_client(nullptr), _local(nullptr) {};
	};
}