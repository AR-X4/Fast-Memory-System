#ifndef USED_H
#define USED_H

#include "Types.h"
#include "Block.h"

class Free;

class Used
{
public:
	// ------------------------------------------------
	// Do not reorder, change or add data fields
	// ------------------------------------------------

	Used() = delete;
	Used(const Used&) = delete;
	Used& operator = (const Used&) = delete;
	~Used() = default;

	Used(uint32_t BlockSize);

	Used* pUsedNext;       // next used block
	Used* pUsedPrev;       // prev used block
	uint32_t   mBlockSize;       // size of block
	uint8_t    mAboveBlockFree;  // AboveBlock flag:
								 // if(AboveBlock is type free) -> true 
								 // if(AboveBlock is type used) -> false
	uint8_t	    pad0;			 // future use
	uint8_t	    pad1;			 // future use
	Block		mType;			 // block type 
};

#endif 