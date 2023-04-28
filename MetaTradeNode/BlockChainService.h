#pragma once
#include <stomp/WebStompType.h>
#include "MetaTradeClient.h"

namespace metatradenode {
	class BlockChainService
	{
	public:
		void RegisterClient(metatradenode::MetaTradeClient* client);
		friend class metatradenode::MetaTradeClient;
	protected:
		virtual void Init(webstomppp::StompFrame& msg);
		virtual void onTrade(webstomppp::StompFrame& msg) = 0;
		virtual void onSpawn(webstomppp::StompFrame& msg) = 0;
		virtual void onJudge(webstomppp::StompFrame& msg) = 0;
		virtual void onSemiSync(webstomppp::StompFrame& msg) = 0;
		virtual void onSync(webstomppp::StompFrame& msg) = 0;
		metatradenode::MetaTradeClient* _client;
		BlockChainService() :_client(nullptr) {};
	};
}

