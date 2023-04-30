#include <stomp/WebStompClient.h>
#include <sstream>
#include <string>

void webstomppp::WebStompClient::_message_dispatcher(websocketpp::connection_hdl hdl, client::message_ptr msg){
	switch (msg->get_opcode())
	{
	case websocketpp::frame::opcode::TEXT:
		{
			StompFrame stomp_msg(msg->get_payload().c_str());
			switch (stomp_msg.type)
			{
			case StompCommandType::CONNECTED:
				this->OnConnected();
				break;
			case StompCommandType::MESSAGE:
			{
				auto it = _topic_callback_map.find(stomp_msg.header["destination"]);
				if (it != _topic_callback_map.end()) {
					(it->second)(stomp_msg);

					StompAckFrame frame(stomp_msg.header["message-id"].c_str());
					char* buf = nullptr;
					size_t len = 0;
					frame.toByteFrame(buf, len);
					_con->send(buf, len, websocketpp::frame::opcode::TEXT);
				}
				else {
					throw StompException(StompExceptionType::SubcribeTopicNotFoundException);
				}
			}
				break;
			case StompCommandType::RECEIPT:
				if (stomp_msg.header["receipt-id"] == std::string(disconnect_receipt_id)) {
					_is_connected.store(false);
					this->OnDisconnected();
				}
				break;
			default:
				break;
			}
		}
		break;
	default:
		break;
	}
}

void webstomppp::WebStompClient::_on_open(client* c, websocketpp::connection_hdl hdl)
{
	auto tmp = _uri.substr(5);
	auto host = tmp.substr(0, tmp.find_first_of('/'));

	StompConnectFrame frame(host.c_str());
	char* buf = nullptr;
	size_t len = 0;
	frame.toByteFrame(buf, len);
	_con->send(buf, len, websocketpp::frame::opcode::TEXT);
}

webstomppp::WebStompClient::WebStompClient()
{
	// Set logging to be pretty verbose (everything except message payloads)
	_ws_client.clear_access_channels(websocketpp::log::alevel::all);
	_ws_client.clear_error_channels(websocketpp::log::elevel::all);

	// Initialize ASIO
	_ws_client.init_asio();
}

webstomppp::WebStompClient::~WebStompClient() {
	if (_is_connected.load()) {
		_ws_client.stop_perpetual();
		_ws_client.close(_con->get_handle(), websocketpp::close::status::going_away, "", _ec);
	}
}

void webstomppp::WebStompClient::Connect(const char* uri)
{
	_uri = uri;
	_con = _ws_client.get_connection(uri, _ec);
	if (_ec) {
		throw StompException(StompExceptionType::ConnectFailedException);
	}

	// Register our message handler
	_con->set_open_handler(websocketpp::lib::bind(&WebStompClient::_on_open, this, &_ws_client, websocketpp::lib::placeholders::_1));
	_con->set_message_handler(websocketpp::lib::bind(&WebStompClient::_message_dispatcher, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));


	// Note that connect here only requests a connection. No network messages are
	// exchanged until the event loop starts running in the next line.
	_ws_client.connect(_con);
}

void webstomppp::WebStompClient::Run()
{
	_ws_client.run();
}

void webstomppp::WebStompClient::Subscribe(const char* destination, webstomppp::callback_func callback){
	auto it = this->_topic_id_map.find(destination);
	if (it != _topic_id_map.end()) return;

	StompSubscribeFrame frame(destination, subscribe_id_gen);
	char* buf = nullptr;
	size_t len = 0;
	frame.toByteFrame(buf, len);
	_con->send(buf, len, websocketpp::frame::opcode::TEXT);

	_topic_id_map.insert(std::make_pair(destination, subscribe_id_gen++));
	_topic_callback_map.insert(std::make_pair(destination, callback));
}
void webstomppp::WebStompClient::Unsubscribe(const char* destination)
{
	auto it = this->_topic_id_map.find(destination);
	if (it != _topic_id_map.end()) return;

	StompUnsubscribeFrame frame(it->second);
	char* buf = nullptr;
	size_t len = 0;
	frame.toByteFrame(buf, len);
	_con->send(buf, len, websocketpp::frame::opcode::TEXT);

	_topic_id_map.erase(destination);
	_topic_callback_map.erase(destination);
}
void webstomppp::WebStompClient::Disconnect()
{
	StompDisconnectFrame frame;
	char* buf = nullptr;
	size_t len = 0;
	frame.toByteFrame(buf, len);
	_con->send(buf, len, websocketpp::frame::opcode::TEXT);
	while (true) {
		if (!_is_connected.load()) {
			_con->close(websocketpp::close::status::normal, "close");
			break;
		}
	}
}
void webstomppp::WebStompClient::Send(StompSendFrame& send_msg) {
	char* buf = nullptr;
	size_t len = 0;
	send_msg.toByteFrame(buf, len);
	_con->send(buf, len, websocketpp::frame::opcode::TEXT);
}
