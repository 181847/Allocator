#pragma once
#include "Allocator.h"

namespace allocator
{

class ALLOCATOR_API StackAllocator
	:public Allocator
{
public:
	StackAllocator(size_t size, void * start);
	virtual ~StackAllocator();

	void * allocate(size_t size, u8 alignment);
	void deallocate(void * addr);

protected:
	void * _curr_pos;
#ifdef _DEBUG
	void * _prev_pos;
#endif

	struct AllocationHeader
	{
#ifdef _DEBUG
		void * prevAddress;
#endif
		u8 adjustment;
	};
};

}// namespace allocator

