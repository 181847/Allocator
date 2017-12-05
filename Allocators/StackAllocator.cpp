#include "StackAllocator.h"

namespace allocator
{


StackAllocator::StackAllocator(size_t size, void * start)
	:Allocator(size, start), _curr_pos(start)
{
#ifdef _DEBUG
	_prev_pos = nullptr;
#endif // _DEBUG
	ASSERT(size > 0);
}


StackAllocator::~StackAllocator()
{
	_curr_pos = nullptr;
#ifdef _DEBUG
	_prev_pos = nullptr;
#endif // _DEBUG
}

void * StackAllocator::allocate(size_t size, u8 alignment)
{
	ASSERT(size > 0);
	u8 adjustment = PointerMath::alignForwardAdjustmentWithHeader(
		_curr_pos, alignment, sizeof(AllocationHeader));

	if (_used_memory + adjustment + size > _size)
		return nullptr;

	// the returned address
	void * p = PointerMath::add(_curr_pos, adjustment);
	// the AllocationHeader
	auto * pAlcHeader = reinterpret_cast<AllocationHeader*>(
		PointerMath::substract(p, sizeof(AllocationHeader)));
	pAlcHeader->adjustment = adjustment;
#ifdef _DEBUG
	pAlcHeader->prevAddress = _prev_pos;
	_prev_pos = p;
#endif // _DEBUG

	_curr_pos = PointerMath::add(p, size);
	_used_memory += adjustment + size;
	++_num_allocations;

	return p;
}

void StackAllocator::deallocate(void * addr)
{
#ifdef _DEBUG
	ASSERT(addr == _prev_pos);
#endif
	auto * pAlcHeader = reinterpret_cast<AllocationHeader*>(
		PointerMath::substract(addr, sizeof(AllocationHeader)));

	size_t adjustment = pAlcHeader->adjustment;

#ifdef _DEBUG
	_prev_pos = pAlcHeader->prevAddress;
#endif

	_used_memory -= adjustment + PointerMath::substract(_curr_pos, addr);
	--_num_allocations;
	_curr_pos = PointerMath::substract(addr, pAlcHeader->adjustment);
}

}// namespace allocator

