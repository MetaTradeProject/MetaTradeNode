#include <stomp/WebStompType.h>
#include <sstream>

const char* webstomppp::disconnect_receipt_id = "4242";

webstomppp::StompFrame::StompFrame(std::string raw_str)
{
	std::stringstream ss;
	ss << raw_str;

	std::string command;
	std::getline(ss, command);

	if (command == "MESSAGE") {
		type = StompCommandType::MESSAGE;
	}
	else if (command == "ERROR") {
		type = StompCommandType::ERROR_FRAME;
	}
	else if (command == "CONNECTED") {
		type = StompCommandType::CONNECTED;
	}
	else if (command == "RECEIPT") {
		type = StompCommandType::RECEIPT;
	}
	else {
		type = StompCommandType::UNKNOWN;
	}

	while (true) {
		std::string line;
		std::getline(ss, line);

		if (line == "") {
			break;
		}

		auto tmp = line.find(':');
		std::string key = line.substr(0, tmp);
		std::string value = line.substr(tmp + 1);

		header.emplace(std::make_pair(key, value));
	}

	ss >> body;
}

void webstomppp::StompFrame::toByteFrame(char*& buf, size_t& len)
{
	std::string str = toRawString();
	buf = new char[str.size() + 1];
	strcpy_s(buf, str.size() + 1, str.c_str());
	buf[str.size()] = '\0';
	len = str.size() + 1;
}

std::string webstomppp::StompFrame::toRawString()
{
	std::stringstream ss;
	switch (type)
	{
	case StompCommandType::CONNECT:
		ss << "CONNECT\n";
		break;
	case StompCommandType::CONNECTED:
		ss << "CONNECTED\n";
		break;
	case StompCommandType::SEND:
		ss << "SEND\n";
		break;
	case StompCommandType::SUBSCRIBE:
		ss << "SUBSCRIBE\n";
		break;
	case StompCommandType::UNSUBSCRIBE:
		ss << "UNSUBSCRIBE\n";
		break;
	case StompCommandType::ACK:
		ss << "ACK\n";
		break;
	case StompCommandType::NACK:
		ss << "NACK\n";
		break;
	case StompCommandType::DISCONNECT:
		ss << "DISCONNECT\n";
		break;
	case StompCommandType::MESSAGE:
		ss << "MESSAGE\n";
		break;
	case StompCommandType::RECEIPT:
		ss << "RECEIPT\n";
		break;
	case StompCommandType::ERROR_FRAME:
		ss << "ERROR\n";
		break;
	default:
		break;
	}

	for (auto& kv : _raw_header) {
		ss << kv.first << ":" << kv.second << "\n";
	}

	ss << "\n" << body;

	return ss.str();
}
