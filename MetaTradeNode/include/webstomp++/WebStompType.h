#pragma once

#include <string>
#include <unordered_map>
#include <exception>
#include <WinSock2.h>
constexpr auto disconnect_receipt_id = "disconnect-42";

namespace webstomppp {
	using StompHeaderKeyValue = std::pair<std::string, std::string>;
	using StompFrameHeader = std::unordered_map<std::string, std::string>;

	enum class __declspec(dllexport) StompCommandType {
		UNKNOWN,
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

	struct __declspec(dllexport) StompCallbackMsg {
		StompCommandType type{ StompCommandType::MESSAGE };
		const char* body{};
		char header_raw[512];
		StompCallbackMsg(StompFrameHeader& header, const char* body);
	};

	struct __declspec(dllexport) StompFrame {
		StompCommandType type{ StompCommandType::CONNECT };
		StompFrameHeader header {};
		std::string body {};

		StompFrame() = default;
		StompFrame(const char* raw_str);
		std::string toRawString();
		friend class WebStompClient;
	protected:
		static void toByteFrame(const char* raw_str, char*& buf, size_t& len);
		std::vector<StompHeaderKeyValue> _raw_header{};
	};

	struct __declspec(dllexport) StompSendFrame : StompFrame {
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

	struct __declspec(dllexport) StompJsonSendFrame final : StompSendFrame {
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

	struct __declspec(dllexport) StompTextSendFrame final : StompSendFrame {
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

	struct __declspec(dllexport) StompSubscribeFrame final : StompFrame {
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

	struct __declspec(dllexport) StompUnsubscribeFrame final : StompFrame {
		StompUnsubscribeFrame(uint64_t id) {
			type = StompCommandType::UNSUBSCRIBE;
			_raw_header.emplace_back(StompHeaderKeyValue("id", std::to_string(id)));

			body = "";
		};
		StompUnsubscribeFrame() = delete;
	};

	struct __declspec(dllexport) StompConnectFrame final : StompFrame {
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

	struct __declspec(dllexport) StompAckFrame final : StompFrame {
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

	struct __declspec(dllexport) StompDisconnectFrame final : StompFrame {
		StompDisconnectFrame() {
			type = StompCommandType::DISCONNECT;
			_raw_header.emplace_back(StompHeaderKeyValue("receipt", disconnect_receipt_id));
		};
	};

	enum class __declspec(dllexport) StompExceptionType {
		UnknownException,
		ConnectFailedException,
		ReceiveErrorFrameException,
		SubcribeTopicNotFoundException
	};

	class __declspec(dllexport) StompException : public std::exception {
		StompExceptionType _type{ StompExceptionType::UnknownException };
	public:
		StompException(StompExceptionType type) :_type(type) {};
		const char* what()  const noexcept override { 
			return "Ooops!\n";
		}
	};
};

