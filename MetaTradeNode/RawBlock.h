#pragma once
#include"MetaTradeClient.h"
#include"Trade.h"
#include"Block.h"

class RawBlock {
private:
	std::vector<Trade> BlockBody;
	int proof;
public:
	std::vector<Trade> getBlockBody();
	int getProof();
};
