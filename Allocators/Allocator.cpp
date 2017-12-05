#include "Allocator.h"

namespace allocator
{

Allocator::Allocator(size_t size, void * start)
	:_start(start), _size(size), _used_memory(0), _num_allocations(0)
{
	ASSERT(size > 0);
}

Allocator::~Allocator()
{
	ASSERT(_num_allocations == 0 && _used_memory == 0);
	_size = 0;
	_start = nullptr;
}

void * Allocator::getStart() const
{
	return _start;
}

size_t Allocator::getSize() const
{
	return _size;
}

size_t Allocator::getUsedMemory() const
{
	return _used_memory;
}

size_t Allocator::getNumAllocations() const
{
	return _num_allocations;
}

}