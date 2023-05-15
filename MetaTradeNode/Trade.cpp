#include "Trade.h"
#include <sstream>

std::string Trade::getHash()
{ 
	std::stringstream ss;
	ss << senderAddress << receiverAddress << amount << commission << timestamp;
	return CryptoUtils::GetSha256(ss.str().c_str());

}
