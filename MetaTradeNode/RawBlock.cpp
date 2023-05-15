#include"RawBlock.h"

std::vector<Trade> RawBlock::getBlockBody()
{
	return BlockBody;
}

int RawBlock::getProof()
{
	return proof;
}
