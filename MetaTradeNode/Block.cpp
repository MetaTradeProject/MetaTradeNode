#include"Block.h"

void Block::calMerkleHash()
{
	merkleHash = "";
	std::deque<std::string> hashTree;
	for (int i = 0; i < BlockBody.size(); i++) {
		hashTree.push_front(BlockBody[i].getHash());
	}
	while (1) {
		int sz = hashTree.size();
		if (sz == 1) break;
		if (sz % 2 != 0) {
			hashTree.push_back(hashTree.back());
		}
		int target = hashTree.size() / 2;
		for (int i = 0; i < target; i++) {
			std::string fir = hashTree.front();
			hashTree.pop_front();
			std::string sec = hashTree.front();
			hashTree.pop_front();
			std::stringstream ss;
			ss << fir << sec;
			hashTree.push_back(CryptoUtils::GetSha256(ss.str().c_str()));
		}
	}
	merkleHash = hashTree.front();
}

Block::Block(std::string prev_hash, int proof_level) {
	this->prevHash = prev_hash;
	this->proofLevel = proof_level;
}
int Block::getProofLevel(){
	return proofLevel;
}
std::vector<Trade> Block::getBlockBody(){
	return BlockBody;
}
double Block::getBlockCommision(){
	double ret = 0;
	for (int i = 0; i < BlockBody.size(); i++) {
		ret += BlockBody[i].commission;
	}
	return ret;
}
std::string Block::getMerkleHash(){
	return merkleHash;
}
std::string Block::getPrevHash(){
	return prevHash;
}
