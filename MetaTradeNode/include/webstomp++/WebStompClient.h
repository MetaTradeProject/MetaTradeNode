#pragma once

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <iostream>
#include <unordered_map>
#include <string>
#include <atomic>
#include <thread>
#include <functional>

#include "WebStompType.h"

namespace webstomppp {
	using ws_client = websocketpp::client<websocketpp::config::asio_client>;
	using wss_client = websocketpp::client<websocketpp::config::asio_tls_client>;
	using message_ptr = websocketpp::config::asio_client::message_type::ptr;
	using stomp_client_ptr = std::shared_ptr<webstomppp::WebStompClient>;

	class STOMP_PUBLIC WebStompClient {
		ws_client _ws_client;
		wss_client _wss_client;
		websocketpp::lib::error_code _ec{};
		ws_client::connection_ptr _con_ws{};
		wss_client::connection_ptr _con_wss{};
		std::string _uri{};
		std::atomic_bool _is_connected{ true };
		bool enable_ssl{false};

		uint64_t subscribe_id_gen{ 0 };
		std::unordered_map<std::string, uint64_t> _topic_id_map{};
		std::unordered_map<std::string, callback_func> _topic_callback_map{};

		void _on_open_ws(ws_client* c, websocketpp::connection_hdl hdl);
		void _on_open_wss(wss_client* c, websocketpp::connection_hdl hdl);
		void _message_dispatcher_ws(websocketpp::connection_hdl hdl, ws_client::message_ptr msg);
		void _message_dispatcher_wss(websocketpp::connection_hdl hdl, wss_client::message_ptr msg);
	public:
		WebStompClient(bool ssl = false);
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