#ifndef FREE_H
#define FREE_H

#include "Types.h"
#include "Block.h"

class Used;

class Free
{
public:
	// ------------------------------------------------
	// Do not reorder, change or add data fields
	// ------------------------------------------------

	Free() = delete;
	Free(const Free&) = delete;
	Free& operator = (const Free&) = delete;
	~Free() = default;

	Free(uint32_t BlockSize);

	Free* pFreeNext;		// next free block
	Free* pFreePrev;		// prev free block
	uint32_t  mBlockSize;		// size of block
	uint8_t   mAboveBlockFree;	// AboveBlock flag:
								// if(AboveBlock is type free) -> true 
								// if(AboveBlock is type used) -> false 
	uint8_t	    pad0;			// future use
	uint8_t	    pad1;			// future use
	Block	 mType;				// block type 
};

#endif 