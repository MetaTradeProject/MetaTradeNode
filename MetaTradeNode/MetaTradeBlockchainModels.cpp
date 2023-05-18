#include "MetaTradeBlockchainModels.h"
#include "CryptoUtils.h"
#include <sstream>
#include <iomanip>
#include <deque>

std::string metatradenode::Trade::getHash() {
    std::stringstream ss;
    ss << senderAddress << receiverAddress << amount << commission << timestamp;
    return CryptoUtils::GetSha256(ss.str().c_str());
}

void metatradenode::Block::calMerkleHash() {
    this->merkle_hash = "";
    std::deque<std::string> hashTree;

    for (auto& trade : block_body) {
        hashTree.push_back(trade.getHash());
    }

    while (true) {
        int sz = hashTree.size();
        if (sz == 1) {
            break;
        }
        if (sz % 2 != 0) {
            hashTree.push_back(hashTree.back());
        }

        int target = hashTree.size() / 2;
        for (int i = 0; i < target; i++) {
            std::string fir = hashTree.front();
            hashTree.pop_front();
            std::string sec = hashTree.front();
            hashTree.pop_front();
            hashTree.push_back(CryptoUtils::GetSha256(fir.append(sec).c_str()));
        }
    }

    this->merkle_hash = hashTree.front();
}

long long metatradenode::Block::getBlockCommision(){
    long long sum = 0;
    for (auto& trade : block_body) {
        sum += trade.commission;
    }
    return sum;
}

std::string metatradenode::Block::getHash()
{
    std::stringstream ss;
    ss << this->prev_hash << this->merkle_hash << this->proof_level << this->proof;
    return CryptoUtils::GetSha256(CryptoUtils::GetSha256(ss.str().c_str()).c_str());
}
