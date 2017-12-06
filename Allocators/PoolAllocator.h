#pragma once
#include "Allocator.h"

namespace allocator
{

class PoolAllocator
	:public Allocator
{
public:
	PoolAllocator(size_t objectSize, u8 alignment, size_t size, void * start);
	virtual ~PoolAllocator();

	void * allocate(size_t size, u8 alignment) override;
	void deallocate(void * addr) override;

protected:
	size_t _objectSize;
	u8 _alignment;
	void * _free_list;
};// class PoolAllocator
}// namespace allocator

