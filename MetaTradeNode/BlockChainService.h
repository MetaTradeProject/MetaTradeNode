#pragma once
#include <webstomp++/WebStompType.h>
#include "MetaTradeClient.h"
#include<string>
#include"Block.h"
#pragma comment(lib , "cJSON.lib")

namespace metatradenode {
	class BlockChainService
	{
	public:
		void RegisterClient(metatradenode::MetaTradeClient* client) { this->_client = client; };
		friend class metatradenode::MetaTradeClient;
		enum Status{ 
			GENESIS, MINING, FINISHED
		};
	protected:
		virtual void Init(webstomppp::StompCallbackMsg msg) { _client->RegisterSubscribe(); };
		virtual void onTrade(webstomppp::StompCallbackMsg msg) = 0;//插入一条trade
		virtual void onSpawn(webstomppp::StompCallbackMsg msg) = 0;//产生rawblock（block的准备工作）
		virtual void onJudge(webstomppp::StompCallbackMsg msg) = 0;//判断raoblock是否正确（收到proof，读取rawblock队列）
		virtual void onSemiSync(webstomppp::StompCallbackMsg msg) = 0;//push新的block
		virtual void onSync(webstomppp::StompCallbackMsg msg) = 0;//储存新的block
		virtual void Stop() = 0;
		metatradenode::MetaTradeClient* _client;
		BlockChainService() :_client(nullptr) {};
	private:
		//Block block = new Block(); 
	};
}

