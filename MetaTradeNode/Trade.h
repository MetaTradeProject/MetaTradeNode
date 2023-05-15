#pragma once
#include <webstomp++/WebStompType.h>
#include "MetaTradeClient.h"
#include "CryptoUtils.h"

struct Trade {
	std::string senderAddress;
	std::string receiverAddress;
	double amount;
	double commission;
	long timestamp;
	std::string signature;
	std::string senderPublicKey;
	std::string description;
	std::string getHash();

};