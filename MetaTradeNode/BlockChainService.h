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
		virtual void onTrade(webstomppp::StompCallbackMsg msg) = 0;//����һ��trade
		virtual void onSpawn(webstomppp::StompCallbackMsg msg) = 0;//����rawblock��block��׼��������
		virtual void onJudge(webstomppp::StompCallbackMsg msg) = 0;//�ж�raoblock�Ƿ���ȷ���յ�proof����ȡrawblock���У�
		virtual void onSemiSync(webstomppp::StompCallbackMsg msg) = 0;//push�µ�block
		virtual void onSync(webstomppp::StompCallbackMsg msg) = 0;//�����µ�block
		virtual void Stop() = 0;
		metatradenode::MetaTradeClient* _client;
		BlockChainService() :_client(nullptr) {};
	private:
		//Block block = new Block(); 
	};
}

