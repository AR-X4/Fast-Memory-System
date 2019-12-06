#include "Mem.h"
#include "Heap.h"
#include "Block.h"
#include "assert.h"
#include "stdio.h"
#include "corecrt_malloc.h"

#define PLACEMENT_NEW_BEGIN	__pragma(push_macro("new")) \
							__pragma(warning( disable : 4291)) // placement new issue

#define PLACEMENT_NEW_END __pragma(pop_macro("new"))

#define HEAP_ALIGNMENT			16
#define HEAP_ALIGNMENT_MASK		(HEAP_ALIGNMENT-1)

#define ALLOCATION_ALIGNMENT		16
#define ALLOCATION_ALIGNMENT_MASK	(ALLOCATION_ALIGNMENT-1)

#ifdef _DEBUG
#define HEAP_HEADER_GUARDS  64
#define HEAP_SET_GUARDS  	uint32_t *pE = (uint32_t *)((uint32_t)pRawMem + HEAP_SIZE); \
								*pE++ = 0xEEEEEEEE; *pE++ = 0xEEEEEEEE; *pE++ = 0xEEEEEEEE; *pE++ = 0xEEEEEEEE; \
								*pE++ = 0xEEEEEEEE; *pE++ = 0xEEEEEEEE; *pE++ = 0xEEEEEEEE; *pE++ = 0xEEEEEEEE; \
								*pE++ = 0xEEEEEEEE; *pE++ = 0xEEEEEEEE; *pE++ = 0xEEEEEEEE; *pE++ = 0xEEEEEEEE;	\
								*pE++ = 0xEEEEEEEE; *pE++ = 0xEEEEEEEE; *pE++ = 0xEEEEEEEE; *pE++ = 0xEEEEEEEE;


#define HEAP_TEST_GUARDS	uint32_t *pE = (uint32_t *)((uint32_t)pRawMem + HEAP_SIZE); \
								assert(*pE++ == 0xEEEEEEEE); assert(*pE++ == 0xEEEEEEEE); \
								assert(*pE++ == 0xEEEEEEEE); assert(*pE++ == 0xEEEEEEEE); \
								assert(*pE++ == 0xEEEEEEEE); assert(*pE++ == 0xEEEEEEEE); \
								assert(*pE++ == 0xEEEEEEEE); assert(*pE++ == 0xEEEEEEEE); \
								assert(*pE++ == 0xEEEEEEEE); assert(*pE++ == 0xEEEEEEEE); \
								assert(*pE++ == 0xEEEEEEEE); assert(*pE++ == 0xEEEEEEEE); \
								assert(*pE++ == 0xEEEEEEEE); assert(*pE++ == 0xEEEEEEEE); \
								assert(*pE++ == 0xEEEEEEEE); assert(*pE++ == 0xEEEEEEEE);
#else
#define HEAP_HEADER_GUARDS  0
#define HEAP_SET_GUARDS  	
#define HEAP_TEST_GUARDS			 
#endif

struct SecretPtr
{
	Free* pFree;
};

Mem::Mem()
{
	this->pHeap = 0;
	this->pRawMem = 0;

	// Get the space for the whole heap
	// Since OS have different alignments... forced it to 16 byte aligned
	pRawMem = _aligned_malloc(HEAP_SIZE + HEAP_HEADER_GUARDS, HEAP_ALIGNMENT);
	HEAP_SET_GUARDS

	// verify alloc worked
	assert(pRawMem != 0);

	// Guarantee alignemnt
	assert(((uint32_t)pRawMem & HEAP_ALIGNMENT_MASK) == 0x0);

	PLACEMENT_NEW_BEGIN
	#undef new

		Heap* p = new(pRawMem) Heap(pRawMem);

	PLACEMENT_NEW_END

	// update it
	this->pHeap = p;
}

Mem::~Mem()
{
	// do not modify this function
	HEAP_TEST_GUARDS
	_aligned_free(this->pRawMem);
}

void Mem::initialize()
{
	Heap* pHeapTmp = this->pHeap;
	Free* pFreeStart = (Free*)(pHeapTmp + 1);
	Free* pFreeEnd = pFreeStart + 1;
	uint32_t BlockSize = (uint32_t)pHeapTmp->mStats.heapBottomAddr - (uint32_t)pFreeEnd;

	PLACEMENT_NEW_BEGIN
	#undef new
		Free* pFree = new(pFreeStart) Free(BlockSize);
	PLACEMENT_NEW_END

		pHeapTmp->pFreeHead = pFreeStart;
	pHeapTmp->pNextFit = pFree;
	pHeapTmp->mStats.currNumFreeBlocks += 1;
	pHeapTmp->mStats.currFreeMem = BlockSize;
}

Heap* Mem::getHeap()
{
	return this->pHeap;
}

void* Mem::malloc(const uint32_t size)
{
	Free* pFree = this->FindFreeBlock(size);
	void* pUsedBlock = nullptr;

	if (pFree != nullptr)
	{
		if (pFree->mBlockSize == size)
		{
			this->RemoveFreeBlock(pFree);
			this->RemoveFreeAdjustStats(pFree);
			this->pHeap->pNextFit = this->pHeap->pFreeHead;

			PLACEMENT_NEW_BEGIN
			#undef new
				Used* pUsed = new(pFree) Used(pFree->mBlockSize);
			PLACEMENT_NEW_END

				this->AddUsedToFront(pUsed);
			this->AddUsedAdjustStats(pUsed);

			pUsedBlock = this->GetBlockPtr(pUsed);
			assert(pUsed != nullptr);
		}
		else if (pFree->mBlockSize > size)//coalescing
		{
			this->RemoveFreeBlock(pFree);
			uint32_t newFreeBlockSize = pFree->mBlockSize - size - 0x10;
			Free* pPrevTmp0 = pFree->pFreePrev;
			Free* pNextTmp0 = pFree->pFreeNext;

			PLACEMENT_NEW_BEGIN
			#undef new
				Used* pUsed = new(pFree) Used(size);
			PLACEMENT_NEW_END

			//update used stats
			this->AddUsedToFront(pUsed);
			this->AddUsedAdjustStats(pUsed);

			//create Free with remaning space
			Free* pFreeNewHDR = (Free*)((uint32_t)(pUsed + 1) + pUsed->mBlockSize);
			PLACEMENT_NEW_BEGIN
			#undef new
				Free* pFree2 = new(pFreeNewHDR) Free(newFreeBlockSize);
			PLACEMENT_NEW_END

				Free* pEnd = (Free*)((uint32_t)(pFree2 + 1) + pFree2->mBlockSize);
			SecretPtr* pSecret = (SecretPtr*)(((uint32_t)pEnd - sizeof(SecretPtr)));
			pSecret->pFree = pFree2;

			if (this->pHeap->pFreeHead == nullptr)
			{
				this->pHeap->pFreeHead = pFree2;
			}

			pFree2->pFreePrev = pPrevTmp0;
			pFree2->pFreeNext = pNextTmp0;
			if (pPrevTmp0 != nullptr) {
				pPrevTmp0->pFreeNext = pFree2;
			}
			if (pNextTmp0 != nullptr) {
				pNextTmp0->pFreePrev = pFree2;
			}

			this->pHeap->pNextFit = pFree2;
			this->pHeap->mStats.currFreeMem -= pUsed->mBlockSize + 0x10;

			pUsedBlock = this->GetBlockPtr(pUsed);
			assert(pUsed != nullptr);
		}
	}
	else
	{
		printf("Malloc Failed!!!!!!!!!!!!\n");
	}
	return pUsedBlock;
}

Free* Mem::FindFreeBlock(const uint32_t size) const
{
	Free* pFreeTmp = this->pHeap->pNextFit;
	Free* pFreeTmp2 = this->pHeap->pNextFit->pFreePrev;

	Free* pFreeFound = nullptr;
	while (pFreeTmp != pFreeTmp2)
	{
		if (pFreeTmp->mBlockSize >= size)
		{
			pFreeFound = pFreeTmp;
			break;
		}

		if (pFreeTmp->pFreeNext == nullptr) {
			pFreeTmp = this->pHeap->pFreeHead;
			pFreeTmp2 = this->pHeap->pNextFit;
		}
		else {
			pFreeTmp = pFreeTmp->pFreeNext;
		}
	}
	return pFreeFound;
}

void Mem::RemoveFreeBlock(const Free* pFree) const
{
	assert(pFree != nullptr);
	if (pFree->pFreeNext == nullptr && pFree->pFreePrev == nullptr)
	{
		this->pHeap->pFreeHead = nullptr;
	}
	else if (pFree->pFreeNext != nullptr && pFree->pFreePrev == nullptr)
	{
		pFree->pFreeNext->pFreePrev = nullptr;
		this->pHeap->pFreeHead = pFree->pFreeNext;
	}
	else if (pFree->pFreeNext == nullptr && pFree->pFreePrev != nullptr)
	{
		pFree->pFreePrev->pFreeNext = nullptr;
	}
	else if (pFree->pFreeNext != nullptr && pFree->pFreePrev != nullptr)
	{
		pFree->pFreeNext->pFreePrev = pFree->pFreePrev;
		pFree->pFreePrev->pFreeNext = pFree->pFreeNext;
	}
}

void Mem::RemoveFreeAdjustStats(const Free* pFree) const
{
	assert(pFree != nullptr);
	this->pHeap->mStats.currFreeMem -= pFree->mBlockSize;
	this->pHeap->mStats.currNumFreeBlocks--;
}

void Mem::AddUsedToFront(Used* pUsed) const
{
	assert(pUsed != nullptr);
	pUsed->pUsedPrev = nullptr;
	if (this->pHeap->pUsedHead == nullptr) {
		this->pHeap->pUsedHead = pUsed;
	}
	else if (this->pHeap->pUsedHead->pUsedNext == nullptr) {
		pUsed->pUsedNext = this->pHeap->pUsedHead;
		this->pHeap->pUsedHead->pUsedPrev = pUsed;
		this->pHeap->pUsedHead = pUsed;
	}
	else
	{
		this->pHeap->pUsedHead->pUsedPrev = pUsed;
		pUsed->pUsedNext = this->pHeap->pUsedHead;
		this->pHeap->pUsedHead = pUsed;
	}
}

void Mem::AddUsedAdjustStats(const Used* pUsed) const
{
	assert(pUsed != nullptr);
	this->pHeap->mStats.currUsedMem += pUsed->mBlockSize;
	this->pHeap->mStats.currNumUsedBlocks += 1;
	if (this->pHeap->mStats.peakNumUsed < this->pHeap->mStats.currNumUsedBlocks) {
		this->pHeap->mStats.peakNumUsed = this->pHeap->mStats.currNumUsedBlocks;
	}
	if (this->pHeap->mStats.peakUsedMemory < this->pHeap->mStats.currUsedMem) {
		this->pHeap->mStats.peakUsedMemory = this->pHeap->mStats.currUsedMem;
	}
}

void* Mem::GetBlockPtr(const Used* pUsed) const
{
	assert(pUsed != nullptr);
	void* p = (void*)(pUsed + 1);
	return p;
}

void Mem::RemoveUsedBlock(const Used* pUsedBlock) const
{
	assert(pUsedBlock != nullptr);

	if (pUsedBlock->pUsedNext == nullptr && pUsedBlock->pUsedPrev == nullptr)
	{
		this->pHeap->pUsedHead = nullptr;
	}
	else if (pUsedBlock->pUsedNext != nullptr && pUsedBlock->pUsedPrev == nullptr)
	{
		pUsedBlock->pUsedNext->pUsedPrev = nullptr;
		this->pHeap->pUsedHead = pUsedBlock->pUsedNext;
	}
	else if (pUsedBlock->pUsedNext == nullptr && pUsedBlock->pUsedPrev != nullptr)
	{
		pUsedBlock->pUsedPrev->pUsedNext = nullptr;
	}
	else if (pUsedBlock->pUsedNext != nullptr && pUsedBlock->pUsedPrev != nullptr)
	{
		pUsedBlock->pUsedPrev->pUsedNext = pUsedBlock->pUsedNext;
		pUsedBlock->pUsedNext->pUsedPrev = pUsedBlock->pUsedPrev;
	}

	this->pHeap->mStats.currUsedMem -= pUsedBlock->mBlockSize;
	this->pHeap->mStats.currNumUsedBlocks -= 1;
}

void Mem::AddFreeBlock(Used* pUsed) const
{
	assert(pUsed != nullptr);
	bool freeAbove = false;
	bool freeBelow = false;

	//check if beloew free
	if (((uint32_t)(pUsed + 1) + pUsed->mBlockSize) < (uint32_t)this->pHeap->mStats.heapBottomAddr)
	{
		Free* pCheck = (Free*)((uint32_t)(pUsed + 1) + pUsed->mBlockSize);
		if (pCheck->mType == Block::Free) {
			freeBelow = true;
		}
	}

	//check if above free... 
	if (pUsed->mAboveBlockFree == 1)
	{
		SecretPtr* pCheck2 = (SecretPtr*)((uint32_t)pUsed - sizeof(SecretPtr));
		Free* pCheck3 = pCheck2->pFree;
		if (pCheck3->mType == Block::Free) {
			freeAbove = true;
		}
	}

	//create the free block
	Free* pFreeStart = (Free*)(pUsed);
	uint32_t BlockSize = pUsed->mBlockSize;

	PLACEMENT_NEW_BEGIN
	#undef new
		Free* pFree = new(pFreeStart) Free(BlockSize);
	PLACEMENT_NEW_END

		//create secret ptr at bottom of free block
		Free* pEnd = (Free*)((uint32_t)(pFree + 1) + pFree->mBlockSize);
	SecretPtr* pSecret = (SecretPtr*)((uint32_t)pEnd - sizeof(SecretPtr));
	pSecret->pFree = pFree;

	//set below above free flag
	if (((uint32_t)(pFree + 1) + pFree->mBlockSize) < (uint32_t)this->pHeap->mStats.heapBottomAddr)
	{
		Used* pBelow = (Used*)((uint32_t)(pFree + 1) + pFree->mBlockSize);
		pBelow->mAboveBlockFree = true;
	}

	//conditions check and ptrs adjust
	if (freeAbove == false && freeBelow == false)
	{
		if (this->pHeap->pFreeHead == nullptr)
		{
			this->pHeap->pFreeHead = pFree;
			this->pHeap->pNextFit = pFree;
		}
		else if (this->pHeap->pFreeHead->pFreeNext != nullptr)
		{
			Free* pFreeTmp = this->pHeap->pFreeHead->pFreeNext;
			while (pFreeTmp != nullptr)
			{
				if (pFreeTmp > pFree) {
					pFree->pFreePrev = pFreeTmp->pFreePrev;
					pFreeTmp->pFreePrev->pFreeNext = pFree;

					pFree->pFreeNext = pFreeTmp;
					pFreeTmp->pFreePrev = pFree;
					break;
				}
				pFreeTmp = pFreeTmp->pFreeNext;
			}
		}
		else {
			if (this->pHeap->pFreeHead > pFree) {
				this->pHeap->pFreeHead->pFreePrev = pFree;
				pFree->pFreeNext = this->pHeap->pFreeHead;
				this->pHeap->pFreeHead = pFree;

			}
			else {
				this->pHeap->pFreeHead->pFreeNext = pFree;
				pFree->pFreePrev = this->pHeap->pFreeHead;
			}
		}
		this->pHeap->mStats.currNumFreeBlocks += 1;
		this->pHeap->mStats.currFreeMem += BlockSize;
	}

	else if (freeAbove == false && freeBelow == true) {

		Free* pBelow2 = (Free*)((uint32_t)(pUsed + 1) + pUsed->mBlockSize);
		Free* pBelowTmpNext = pBelow2->pFreeNext;
		Free* pBelowTmpPrev = pBelow2->pFreePrev;

		uint32_t size = pUsed->mBlockSize + pBelow2->mBlockSize + 0x10;
		PLACEMENT_NEW_BEGIN
		#undef new
			Free* p = new(pFreeStart) Free(size);
		PLACEMENT_NEW_END

			Free* pEnd2 = (Free*)((uint32_t)(p + 1) + p->mBlockSize);
		SecretPtr* pSecret2 = (SecretPtr*)((uint32_t)pEnd2 - sizeof(SecretPtr));
		pSecret2->pFree = p;

		p->pFreePrev = pBelow2->pFreePrev;
		p->pFreeNext = pBelow2->pFreeNext;
		if (pBelowTmpNext != nullptr) {
			pBelowTmpNext->pFreePrev = p;
		}
		if (pBelowTmpPrev != nullptr) {
			pBelowTmpPrev->pFreeNext = p;
		}

		if (p < this->pHeap->pFreeHead) {
			p->pFreeNext = this->pHeap->pFreeHead->pFreeNext;
			p->pFreePrev = nullptr;
			this->pHeap->pFreeHead = p;
		}

		if (p->pFreeNext == nullptr) {
			this->pHeap->pNextFit = p;
		}
		this->pHeap->mStats.currFreeMem += size - pBelow2->mBlockSize;
	}

	else if (freeAbove == true && freeBelow == false) {

		SecretPtr* pSecretAbove = (SecretPtr*)((uint32_t)pUsed - sizeof(SecretPtr));
		Free* pAbove = pSecretAbove->pFree;
		Free* pPrevTmp = pAbove->pFreePrev;
		Free* pNextTmp = pAbove->pFreeNext;

		uint32_t size2 = pUsed->mBlockSize + pAbove->mBlockSize + 0x10;
		PLACEMENT_NEW_BEGIN
		#undef new
			Free* p2 = new(pAbove) Free(size2);
		PLACEMENT_NEW_END

			Free* pEnd3 = (Free*)((uint32_t)(p2 + 1) + p2->mBlockSize);
		SecretPtr* pSecret3 = (SecretPtr*)(((uint32_t)pEnd3 - sizeof(SecretPtr)));
		pSecret3->pFree = p2;

		p2->pFreePrev = pPrevTmp;
		p2->pFreeNext = pNextTmp;

		if (p2 < this->pHeap->pFreeHead) {
			p2->pFreeNext = this->pHeap->pFreeHead->pFreeNext;
			p2->pFreePrev = nullptr;
			this->pHeap->pFreeHead = p2;
		}
		this->pHeap->mStats.currFreeMem += pUsed->mBlockSize + 0x10;
	}

	else if (freeAbove == true && freeBelow == true) {

		SecretPtr* pSecretAbove2 = (SecretPtr*)((uint32_t)pUsed - sizeof(SecretPtr));
		Free* pAbove2 = pSecretAbove2->pFree;
		Free* pPrevTmp2 = pAbove2->pFreePrev;

		Free* pBelow3 = (Free*)((uint32_t)(pUsed + 1) + pUsed->mBlockSize);
		Free* pNextTmp2 = pBelow3->pFreeNext;

		uint32_t size3 = pUsed->mBlockSize + pAbove2->mBlockSize + pBelow3->mBlockSize + 0x20;

		PLACEMENT_NEW_BEGIN
		#undef new
			Free* p3 = new(pAbove2) Free(size3);
		PLACEMENT_NEW_END

		Free* pEnd4 = (Free*)((uint32_t)(p3 + 1) + p3->mBlockSize);
		SecretPtr* pSecret3 = (SecretPtr*)(((uint32_t)pEnd4 - sizeof(SecretPtr)));
		pSecret3->pFree = p3;

		p3->pFreePrev = pPrevTmp2;
		p3->pFreeNext = pNextTmp2;

		if (p3->pFreePrev == nullptr && p3->pFreeNext == nullptr) {
			this->pHeap->mStats.currNumFreeBlocks = 1;
			this->pHeap->pNextFit = p3;
		}

		if (p3 < this->pHeap->pFreeHead) {
			p3->pFreeNext = this->pHeap->pFreeHead->pFreeNext;
			p3->pFreePrev = nullptr;
			this->pHeap->pFreeHead = p3;
		}
		this->pHeap->mStats.currFreeMem += pUsed->mBlockSize + 0x20;
	}
}

void Mem::free(void* const data)
{
	Used* pUsedHdrStart = (Used*)(data)-1;
	this->RemoveUsedBlock(pUsedHdrStart);
	this->AddFreeBlock(pUsedHdrStart);
}