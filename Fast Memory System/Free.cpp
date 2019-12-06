#include "Used.h"
#include "Free.h"
#include "Block.h"

Free::Free(uint32_t BlockSize)
	: pFreeNext(nullptr), pFreePrev(nullptr), mBlockSize(BlockSize), mAboveBlockFree(false), pad0(0), pad1(0), mType(Block::Free)
{

}