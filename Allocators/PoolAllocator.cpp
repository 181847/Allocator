#include "PoolAllocator.h"

namespace allocator
{



allocator::PoolAllocator::PoolAllocator(size_t objectSize, u8 alignment, size_t size, void * start)
	:Allocator(size, start), _objectSize(objectSize), _alignment(alignment)
{
	ASSERT(objectSize >= sizeof(void*));

	u8 adjustment = PointerMath::alignForwardAdjustment(start, alignment);

	_free_list = PointerMath::add(start, adjustment);

	size_t totalObjectCount = (size - adjustment) / objectSize;


	void ** pcurr = reinterpret_cast<void**>(_free_list);
	// iterate all the objectSize block except the last one
	for (size_t i = 0; i < totalObjectCount - 1; ++i)
	{
		*pcurr = PointerMath::add(pcurr, objectSize);

		// pcurr      *pcurr
		//  |           |
		//  |           |
		//  |           |
		//  V  objSize  V
		//  ----------------------------....
		//  |  *pcurr  ||   next   ||
		//  |----------||----------||---....
		
		pcurr = reinterpret_cast<void**>(*pcurr);
	}
	*pcurr = nullptr;

}// PoolAllocator constructor

PoolAllocator::~PoolAllocator()
{
	_free_list = nullptr;
	_alignment = 0;
	_objectSize = 0;
}// PoolAllocator deconstructor

void * PoolAllocator::allocate(size_t size, u8 alignment)
{
	ASSERT(alignment == _alignment && size == _objectSize);

	if (_free_list)
	{
		void * pret = _free_list;
		_free_list = *reinterpret_cast<void**>(_free_list);

		_used_memory += size;
		++_num_allocations;
		return pret;
	}
	else
	{
		return nullptr;
	}
}// PoolAllocator::allocate

void PoolAllocator::deallocate(void * addr)
{
	void ** paddr = reinterpret_cast<void**>(addr);
	*paddr = _free_list;
	_free_list = addr;

	_used_memory -= _objectSize;
	--_num_allocations;
}// PoolAllocator::deallocate



}// namespace allocator
