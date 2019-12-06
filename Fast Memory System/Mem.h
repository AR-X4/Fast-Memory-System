#ifndef MEM_H
#define MEM_H

#include "Heap.h"

class Mem
{
public:
	static const unsigned int HEAP_SIZE = (50 * 1024);

public:
	Mem();
	Mem(const Mem&) = delete;
	Mem& operator = (const Mem&) = delete;
	~Mem();

	Heap* getHeap();
	//void dump();

	void free(void* const data);
	void* malloc(const uint32_t size);
	void initialize();

private:
	Free* FindFreeBlock(const uint32_t size) const;
	void RemoveFreeBlock(const Free* pFree) const;
	void RemoveFreeAdjustStats(const Free* pFree) const;
	void AddUsedToFront(Used* pUsed) const;
	void AddUsedAdjustStats(const Used* pUsed) const;
	void* GetBlockPtr(const Used* pUsed) const;
	void RemoveUsedBlock(const Used* pUsed) const;
	void AddFreeBlock(Used* pUsed) const;//for free perfect size

private:
	Heap* pHeap;
	void* pRawMem;

};

#endif 
