#include "FreeListAllocator.h"

namespace allocator
{



FreeListAllocator::FreeListAllocator(size_t size, void * start)
	:Allocator(size, start), _free_blocks(reinterpret_cast<FreeBlock*>(start))
{
	ASSERT(size > 0);
	_free_blocks->next = nullptr;
	_free_blocks->size = size;
}


FreeListAllocator::~FreeListAllocator()
{
	ASSERT(_used_memory == 0 && _num_allocations == 0);
	_free_blocks = nullptr;
}

void * FreeListAllocator::allocate(size_t size, u8 alignment)
{
	ASSERT(size > 0);
	PFreeBlock	freeBlock	= _free_blocks;
	PFreeBlock	prevBlock	= nullptr;
	u8			adjustment	= 0;
	size_t		totalSize	= 0;

	// find the a
	while (freeBlock)
	{
		adjustment = PointerMath::alignForwardAdjustmentWithHeader(
			freeBlock, alignment, sizeof(AllocationHeader));
		totalSize = size + adjustment;

		// find the first available FreeBlock.
		if (totalSize < freeBlock->size)
		{
			prevBlock = freeBlock;
			freeBlock = freeBlock->next;
			continue;
		}
		else
		{
			// have find one
			break;
		}
	}

	// exit with nullptr, no desired size,
	// allocation failed.
	if (!freeBlock)
		return nullptr;

	static_assert(sizeof(AllocationHeader) >= sizeof(FreeBlock),
		"AllocationHeader should greater than FreeBlock, because next few code will compare to AllocationHeader, which should be the larger one.");

	// check the remain memory is enough for another FreeBlock
	if (freeBlock->size - totalSize < sizeof(AllocationHeader))
	{// cannot create another FreeBlock
		totalSize = freeBlock->size;
		
		if (prevBlock)
		{
			prevBlock->next = freeBlock->next;
		}
		else
		{
			_free_blocks = freeBlock->next;
		}
	}
	else
	{// enough space for another FreeBlock
		PFreeBlock splitedOne = PointerMath::add(freeBlock, totalSize);
		splitedOne->size = freeBlock->size - totalSize;
		splitedOne->next = freeBlock->next;
	}

	// prepare the returned pointer
	void * pret = PointerMath::add(reinterpret_cast<void*>(freeBlock), adjustment);
	PAllocationHeader header = reinterpret_cast<PAllocationHeader>(
		PointerMath::substract(pret, sizeof(AllocationHeader)));
	header->size = totalSize;
	header->adjustment = adjustment;

	_used_memory += totalSize;
	++_num_allocations;

	return pret;
}

void FreeListAllocator::deallocate(void * addr)
{
	ASSERT(addr);
	auto * header = reinterpret_cast<PAllocationHeader>(
		PointerMath::substract(addr, sizeof(AllocationHeader)));
	size_t	size		= header->size;
	u8		adjustment	= header->adjustment;

	// deallocated Block boundaries.
	PFreeBlock blockStart = reinterpret_cast<PFreeBlock>(
		PointerMath::substract(addr, adjustment));
	PFreeBlock blockEnd = PointerMath::add(blockStart, size);

	// for iteration
	PFreeBlock prevBlock = nullptr;
	PFreeBlock freeBlock = _free_blocks;

	while ( ! freeBlock )
	{
		if (freeBlock >= blockStart)
			break;
		else
		{
			prevBlock = freeBlock;
			freeBlock = freeBlock->next;
		}
	}

	if (prevBlock)
	{// find inside the freeList
		if (blockStart == PointerMath::add(prevBlock, prevBlock->size))
		{// merge deallocated block into previous block
			prevBlock->size += size;
			// change blockStart, because later
			// we will try to merge the next block into the previous one.
			blockStart = prevBlock;
		}
		else
		{
			blockStart->next = prevBlock->next;
			blockStart->size = size;
		}
	}// end if(prevBlock)
	else
	{// found before the head
		blockStart->next = freeBlock;
		// change the allocator's freeList head.
		_free_blocks = blockStart;
	}// end else(prevBlock)

	// if freeBlock is not nullptr,
	// try to merge freeBlock into blockStart.
	if (blockEnd == freeBlock)
	{
		blockStart->size += freeBlock->size;
	}

	// log memory state
	_used_memory += size;
	--_num_allocations;
}

};// namespace allocator
