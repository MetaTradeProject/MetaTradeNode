#pragma once

#if !defined(__WINDOWS__) && (defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32))
#define __WINDOWS__
#endif

#ifdef __WINDOWS__
#define STOMP_PUBLIC   __declspec(dllexport)
#include <WinSock2.h>
#else
#define STOMP_PUBLIC  
#endif

#include <string>
#include <unordered_map>
#include <exception>
#include <functional>
constexpr auto disconnect_receipt_id = "disconnect-42";

namespace webstomppp {
	using StompHeaderKeyValue = std::pair<std::string, std::string>;
	using StompFrameHeader = std::unordered_map<std::string, std::string>;

	enum class STOMP_PUBLIC StompCommandType {
		UNKNOWN,
		STOMP,
		CONNECT,
		CONNECTED,
		SEND,
		SUBSCRIBE,
		UNSUBSCRIBE,
		ACK,
		NACK,
		DISCONNECT,
		MESSAGE,
		RECEIPT,
		ERROR_FRAME
	};

	struct STOMP_PUBLIC StompCallbackMsg {
		StompCommandType type{ StompCommandType::MESSAGE };
		uint64_t session_id;
		const char* body{};
		char header_raw[1024];
		StompCallbackMsg(StompFrameHeader& header, const char* body, uint64_t session_id = -1, StompCommandType type = StompCommandType::MESSAGE);
	};

	using callback_func = std::function<void(StompCallbackMsg)>;

	struct STOMP_PUBLIC StompFrame {
		StompCommandType type{ StompCommandType::CONNECT };
		StompFrameHeader header {};
		std::string body {};

		StompFrame() = default;
		StompFrame(const char* raw_str);
		void toRawString(char*&);
		friend class WebStompClient;
		friend class WebStompServer;
	protected:
		static void toByteFrame(const char* raw_str, char*& buf, size_t& len);
		std::vector<StompHeaderKeyValue> _raw_header{};
	};

	struct STOMP_PUBLIC StompMessageFrame final : StompFrame{
		StompMessageFrame(const char* destination, const char* subscription, const char* message_id, const char* content, const char* content_type, StompFrameHeader* user_defined_header);
	};

	struct STOMP_PUBLIC StompSendFrame : StompFrame {
		StompSendFrame(const char* destination, const char* content, const char* content_type, StompFrameHeader* user_defined_header) {
			type = StompCommandType::SEND;
			_raw_header.emplace_back(StompHeaderKeyValue("destination", destination));
			_raw_header.emplace_back(StompHeaderKeyValue("content-type", content_type));

			if (user_defined_header != nullptr) {
				for (auto& kv : *user_defined_header) {
					_raw_header.emplace_back(StompHeaderKeyValue(kv.first, kv.second));
				}
			}

			body = content;
		}
	protected:
		StompSendFrame() { 
			type = StompCommandType::SEND; 
		};
	};

	struct STOMP_PUBLIC StompJsonSendFrame final : StompSendFrame {
		StompJsonSendFrame(const char* destination, const char* content, StompFrameHeader* user_defined_header = nullptr): StompSendFrame() {
			_raw_header.emplace_back(StompHeaderKeyValue("destination", destination));
			_raw_header.emplace_back(StompHeaderKeyValue("content-type", "application/json"));

			if (user_defined_header != nullptr) {
				for (auto& kv : *user_defined_header) {
					_raw_header.emplace_back(StompHeaderKeyValue(kv.first, kv.second));
				}
			}

			body = content;
		};
		StompJsonSendFrame() = delete;
	};

	struct STOMP_PUBLIC StompTextSendFrame final : StompSendFrame {
		StompTextSendFrame(const char* destination, const char* content, StompFrameHeader* user_defined_header = nullptr): StompSendFrame() {
			_raw_header.emplace_back(StompHeaderKeyValue("destination", destination));
			_raw_header.emplace_back(StompHeaderKeyValue("content-type", "text/plain"));

			if (user_defined_header != nullptr) {
				for (auto& kv : *user_defined_header) {
					_raw_header.emplace_back(StompHeaderKeyValue(kv.first, kv.second));
				}
			}

			body = content;
		};
		StompTextSendFrame() = delete;
	};

	struct STOMP_PUBLIC StompSubscribeFrame final : StompFrame {
		StompSubscribeFrame(const char* destination, uint64_t id, StompFrameHeader* user_defined_header = nullptr) {
			type = StompCommandType::SUBSCRIBE;
			_raw_header.emplace_back(StompHeaderKeyValue("id", std::to_string(id)));
			_raw_header.emplace_back(StompHeaderKeyValue("destination", destination));
			_raw_header.emplace_back(StompHeaderKeyValue("ack", "client"));

			if (user_defined_header != nullptr) {
				for (auto& kv : *user_defined_header) {
					_raw_header.emplace_back(StompHeaderKeyValue(kv.first, kv.second));
				}
			}
			
			body = "";
		};
		StompSubscribeFrame() = delete;
	};

	struct STOMP_PUBLIC StompUnsubscribeFrame final : StompFrame {
		StompUnsubscribeFrame(uint64_t id) {
			type = StompCommandType::UNSUBSCRIBE;
			_raw_header.emplace_back(StompHeaderKeyValue("id", std::to_string(id)));

			body = "";
		};
		StompUnsubscribeFrame() = delete;
	};

	struct STOMP_PUBLIC StompConnectFrame final : StompFrame {
		StompConnectFrame(const char* host, StompFrameHeader* user_defined_header = nullptr) {
			type = StompCommandType::CONNECT;
			_raw_header.emplace_back(StompHeaderKeyValue("accept-version", "1.2"));
			_raw_header.emplace_back(StompHeaderKeyValue("host", host));

			if (user_defined_header != nullptr) {
				for (auto& kv : *user_defined_header) {
					_raw_header.emplace_back(StompHeaderKeyValue(kv.first, kv.second));
				}
			}

			body = "";
		};
		StompConnectFrame() = delete;
	};

	struct STOMP_PUBLIC StompAckFrame final : StompFrame {
		StompAckFrame(const char* msg_id, StompFrameHeader* user_defined_header = nullptr) {
			type = StompCommandType::ACK;
			_raw_header.emplace_back(StompHeaderKeyValue("id", msg_id));

			if (user_defined_header != nullptr) {
				for (auto& kv : *user_defined_header) {
					_raw_header.emplace_back(StompHeaderKeyValue(kv.first, kv.second));
				}
			}

			body = "";
		};
		StompAckFrame() = delete;
	};

	struct STOMP_PUBLIC StompDisconnectFrame final : StompFrame {
		StompDisconnectFrame() {
			type = StompCommandType::DISCONNECT;
			_raw_header.emplace_back(StompHeaderKeyValue("receipt", disconnect_receipt_id));
		};
	};

	struct STOMP_PUBLIC StompReceiptFrame final : StompFrame{
		StompReceiptFrame(const char* receipt_id) {
			type = StompCommandType::RECEIPT;
			_raw_header.emplace_back(StompHeaderKeyValue("receipt-id", receipt_id));
		};
	};

	struct STOMP_PUBLIC StompConnectedFrame final : StompFrame{
		StompConnectedFrame(const char* version, const char* session, const char* server = nullptr, const char* heart_beat = nullptr) {
			type = StompCommandType::CONNECTED;
			_raw_header.emplace_back(StompHeaderKeyValue("version", version));
			_raw_header.emplace_back(StompHeaderKeyValue("session", session));
			if (server != nullptr) {
				_raw_header.emplace_back(StompHeaderKeyValue("server", server));
			}
			if (heart_beat != nullptr) {
				_raw_header.emplace_back(StompHeaderKeyValue("heart_beat", heart_beat));
			}
		};
	};

	enum class STOMP_PUBLIC StompExceptionType {
		UnknownException,
		ConnectFailedException,
		ReceiveErrorFrameException,
		SubcribeTopicNotFoundException
	};

	class STOMP_PUBLIC StompException : public std::exception {
		StompExceptionType _type{ StompExceptionType::UnknownException };
	public:
		StompException(StompExceptionType type) :_type(type) {};
		const char* what()  const noexcept override { 
			return "Ooops!\n";
		}
	};
};

