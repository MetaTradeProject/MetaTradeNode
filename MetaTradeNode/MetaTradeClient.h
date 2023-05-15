#pragma once
#include <webstomp++/WebStompClient.h>
#pragma comment(lib , "libWebStomp++.lib")

namespace metatradenode {
	extern const char* STOMP_PATH;
	extern const char* SUB_INIT;
	extern const char* SUB_TRADE;
	extern const char* SUB_SPAWN;
	extern const char* SUB_JUDGE;
	extern const char* SUB_SEMI_SYNC;
	extern const char* SUB_SYNC;
	extern const char* POST_TRADE;
	extern const char* POST_PROOF;
	extern const char* POST_AGREE;
	extern const char* POST_SYNC;

	class BlockchainService;
	class LocalService;

	class MetaTradeClient : public webstomppp::WebStompClient
	{
	public:
		enum class Status {
			BORN,
			SEND_INIT,
			SERVICE_INIT,
			SUB_ALL,
			FINISHED
		};
		MetaTradeClient() : _status(Status::BORN), _bc_service(nullptr), _local_service(nullptr), _async_thread(nullptr) {};
		~MetaTradeClient();
		void RegisterService(metatradenode::BlockchainService* service1, metatradenode::LocalService* service2) { this->_bc_service = service1; this->_local_service = service2; };
		void RunSync();
		void RunAsync();
		friend class BlockchainService;
	private:
		std::thread* _async_thread;
		void OnConnected() override;
		void OnDisconnected() override;
		void RegisterSubscribe();
		metatradenode::BlockchainService* _bc_service;
		metatradenode::LocalService* _local_service;
		Status _status;
	};
};


