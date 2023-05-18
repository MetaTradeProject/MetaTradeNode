#pragma once

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <iostream>
#include <unordered_map>
#include <string>
#include <atomic>
#include <thread>
#include <functional>

#include "WebStompType.h"

namespace webstomppp {
	using client = websocketpp::client<websocketpp::config::asio_client>;
	using message_ptr = websocketpp::config::asio_client::message_type::ptr;
	using callback_func = std::function<void(StompCallbackMsg)>;
	using stomp_client_ptr = std::shared_ptr<WebStompClient>;

	class __declspec(dllexport) WebStompClient {
		client _ws_client;
		websocketpp::lib::error_code _ec{};
		client::connection_ptr _con{};
		std::string _uri{};
		std::atomic_bool _is_connected{ true };

		uint64_t subscribe_id_gen{ 0 };
		std::unordered_map<std::string, uint64_t> _topic_id_map{};
		std::unordered_map<std::string, callback_func> _topic_callback_map{};

		void _on_open(client* c, websocketpp::connection_hdl hdl);
		void _message_dispatcher(websocketpp::connection_hdl hdl, client::message_ptr msg);

	public:
		WebStompClient();
		~WebStompClient();
		void Connect(const char* uri);
		void Disconnect();
		void Run();
		void Subscribe(const char* destination, webstomppp::callback_func callback);
		void Unsubscribe(const char* destination);
		void Send(const char* raw_str);
		void SendJson(const char* des, const char* content);
		virtual void OnConnected() {};
		virtual void OnDisconnected() {};
	};
};