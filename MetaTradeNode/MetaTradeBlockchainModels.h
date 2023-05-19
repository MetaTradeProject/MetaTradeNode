#pragma once
#include <string>
#include <vector>
namespace metatradenode {
	struct Trade {
		std::string senderAddress;
		std::string receiverAddress;
		long long amount;
		long long commission;
		long long timestamp;
		std::string signature;
		std::string senderPublicKey;
		std::string description;
		std::string getHash();

	};

	struct Bill {
		char sender[35];
		char receiver[35];
		char id[10];
		long long amount;
		long long commission;
		long long timestamp;
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
		long long getBlockCommision();
		std::string getHash();
	};
}