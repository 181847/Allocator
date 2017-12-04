#pragma once
#include "Allocator.h"
#include "Allocator.h"

namespace allocator
{

class LinearAllocator :
	public Allocator
{
public:
	LinearAllocator(size_t size, void * start);
	DELETE_COPY_CONSTRUCTOR(LinearAllocator)
	virtual ~LinearAllocator();

	void* allocate(size_t size, u8 align) override;
	void deallocate(void * addr) override;

	// to clear all memory.
	void clear();

protected:
	// now avaliable memory address
	void * _curr_pos;
};// class LinearAllocator

}// allocator

