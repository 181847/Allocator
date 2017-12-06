#pragma once
#include "Allocator.h"

namespace allocator
{

class ALLOCATOR_API FreeListAllocator
	:public Allocator
{
public:
	FreeListAllocator(size_t size, void * start);
	virtual~FreeListAllocator();

	void * allocate(size_t size, u8 alignment) override;
	void deallocate(void * addr) override;

#ifdef _DEBUG
public:
#else
private:
#endif
	typedef struct FreeBlock
	{
		size_t size;
		FreeBlock * next;
	}FreeBlock, * PFreeBlock;
	typedef struct AllocationHeader
	{
		size_t size;
		u8 adjustment;
	}AllocationHeader, * PAllocationHeader;

private:
	FreeBlock * _free_blocks;
};

}// namespace allocator

