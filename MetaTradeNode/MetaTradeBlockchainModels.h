#pragma once
#include <string>
#include <vector>
namespace metatradenode {
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

	struct RawBlock {
		std::vector<Trade> block_body;
		int proof_level;
	};

	struct Block {
		std::string prev_hash;
		std::string merkle_hash;
		int proof_level;
		int proof;
		std::vector<Trade> block_body;
		void calMerkleHash();
		double getBlockCommision();
		std::string getHash();
	};
}