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

	class BlockChainService;

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
		MetaTradeClient(const char* wallet_address) : _wallet_address(wallet_address), _status(Status::BORN), _service(nullptr), _async_thread(nullptr) {};
		~MetaTradeClient();
		void RegisterService(metatradenode::BlockChainService* service) { this->_service = service; };
		void RunSync();
		void RunAsync();
		friend class BlockChainService;
	private:
		std::thread* _async_thread;
		void OnConnected() override;
		void OnDisconnected() override;
		void RegisterSubscribe();
		std::string _wallet_address;
		metatradenode::BlockChainService* _service;
		Status _status;
	};
};


