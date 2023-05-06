#pragma once
#include <webstomp++/WebStompType.h>
#include "MetaTradeClient.h"
#pragma comment(lib , "cJSON.lib")

namespace metatradenode {
	class BlockChainService
	{
	public:
		void RegisterClient(metatradenode::MetaTradeClient* client) { this->_client = client; };
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
		BlockChainService() :_client(nullptr) {};
	};
}

