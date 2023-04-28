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
	using callback_func = std::function<void(StompFrame)>;
	using stomp_client_ptr = std::shared_ptr<WebStompClient>;

	class WebStompClient {
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
		void Connect(std::string uri);
		void Disconnect();
		void Run();
		void Subscribe(std::string destination, webstomppp::callback_func callback);
		void Unsubscribe(std::string destination);
		void Send(StompSendFrame& send_msg);
		virtual void OnConnected() {};
		virtual void OnDisconnected() {};
	};
};