#include "Free.h"
#include "Used.h"

Used::Used(uint32_t BlockSize)
	: pUsedNext(nullptr), pUsedPrev(nullptr), mBlockSize(BlockSize), mAboveBlockFree(false), pad0(0), pad1(0), mType(Block::Used)
{

}
