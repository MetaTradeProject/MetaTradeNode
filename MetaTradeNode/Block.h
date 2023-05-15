#pragma once
#include <webstomp++/WebStompType.h>
#include "MetaTradeClient.h"
#include<string>
#include"Trade.h"
#include"CryptoUtils.h"

class Block {
private:
	std::string prevHash;
	std::string merkleHash;
	int proofLevel;
	int proof;
	std::vector<Trade> BlockBody;
	void calMerkleHash();
public:
	Block(std::string prev_hash, int proof_level);
	int getProofLevel();
	std::vector<Trade> getBlockBody();
	double getBlockCommision();
	std::string getMerkleHash();
	std::string getPrevHash();
};