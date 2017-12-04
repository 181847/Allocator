#include "LinearAllocator.h"

namespace allocator
{


LinearAllocator::LinearAllocator(size_t size, void * start)
	:Allocator(size, start), _curr_pos(start)
{
	ASSERT(size > 0);
}


LinearAllocator::~LinearAllocator()
{
	_curr_pos = nullptr;
}

void * LinearAllocator::allocate(size_t size, u8 align)
{
	ASSERT(size > 0);
	u8 adjustment = PointerMath::alignForwardAdjustment(_curr_pos, align);

	// have used out the memory?
	if (_used_memory + size + adjustment > _size)
		return nullptr;

	void * ret = _curr_pos;
	// update current position 
	_curr_pos = PointerMath::add(_curr_pos, size + adjustment);
	_used_memory += (size + adjustment);
	++_num_allocations;
	
	return ret;
}

void LinearAllocator::deallocate(void * addr)
{
	ASSERT(false && "You shouldn't use this function, please use clear()");
}

void LinearAllocator::clear()
{
	_curr_pos = _start;
	_used_memory = 0;
	_num_allocations = 0;
}


}// namespace allocator
