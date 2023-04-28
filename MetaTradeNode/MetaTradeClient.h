#pragma once
#include <stomp/WebStompClient.h>
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

	class MetaTradeClient : protected webstomppp::WebStompClient
	{
	public:
		enum class Status {
			BORN,
			SEND_INIT,
			SERVICE_INIT,
			SUB_ALL,
			FINISHED
		};
		MetaTradeClient(std::string address) : _address(address), _status(Status::BORN), _service(nullptr), _async_thread(nullptr) {};
		~MetaTradeClient() {
			if (_async_thread != nullptr) {
				_async_thread->join();
				delete _async_thread;
			}
		}
		void RegisterService(metatradenode::BlockChainService* service);
		void RunSync();
		void RunAsync();
		friend class BlockChainService;
	private:
		std::thread* _async_thread;
		void OnConnected() override;
		void OnDisconnected() override;
		void RegisterSubscribe();
		std::string _address;
		metatradenode::BlockChainService* _service;
		Status _status;
	};
};


