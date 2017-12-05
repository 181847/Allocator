#pragma once
#include "../../Library/MyTools/UsefulDataType.h"
#include "../../Library/MyTools/MyAssert.h"
#include "../../Library/MyTools/MyTools.h"
#include "../../Library/MyTools/DLLTools.h"
#include "PointerMath.h"
#include "../../Library/MyTools/MetaTools.h"

#ifdef ALLOCATORS_EXPORTS
#define ALLOCATOR_API DLL_EXPORT_API
#else
#ifdef ALLOCATORS_IMPORTS
#define ALLOCATOR_API DLL_IMPORT_API
#else
#define ALLOCATOR_API
#endif
#endif

namespace allocator
{

// this class serve as the interface of all Allocator.
class ALLOCATOR_API Allocator
{
public:
	Allocator(size_t size, void * start);
	DELETE_COPY_CONSTRUCTOR(Allocator)
	virtual ~Allocator();

	virtual void * allocate(size_t size, u8 alignment) = 0;
	virtual void deallocate(void * address) = 0;
	void * getStart() const;
	size_t getSize() const;
	size_t getUsedMemory() const;
	size_t getNumAllocations() const;
	
protected:
	void * _start;
	size_t _size;
	size_t _used_memory;
	size_t _num_allocations;
};

template<typename T>
T* AllocateNew(Allocator& allocator)
{
	return new (allocator.allocate(sizeof(T), alignof(T))) T;
}

template<typename T>
void Deallocate(Allocator& allocator, T * p)
{
	p.~T();
	allocator.deallocate(reinterpret_cast<T*>(p));
}



template<typename T>
T* AllocateArray(Allocator& allocator, size_t length)
{
	const size_t headSize = PointerMath::ALIGN_ARR_HEADER<T>::ret;

	// get the array address,
	// we will jump front head length;
	T* p = headSize + reinterpret_cast<T*>(allocator.allocate(sizeof(T) * (length + headSize), alignof(T)));
	// log length

	// does the allocator return nullptr?
	ASSERT(p - headSize != nullptr && "allocator returned nullptr");

	*(reinterpret_cast<size_t*>(p) - 1) = length;

	// call the constructor
	for (size_t i = 0; i < length; ++i)
	{
		new (p + i) T;
	}

	return p;
}// AllocateArray

template<typename T>
inline void DeallocateArray(Allocator& allocator, T* parray)
{
	const size_t length = *(reinterpret_cast<size_t*>(parray) - 1);

	// call deconstructor for each element
	for (size_t i = 0; i < length; ++i)
	{
		parray[i].~T();
	}
	// don't miss the substract operation,
	// which will offset the array head pointer the the origianl
	// pointer the allocator previously returned.
	allocator.deallocate(parray - ALIGN_ARR_HEADER<T>::reType::ret);
}// DeallocateArray

inline void showAllocator(Allocator & allocator)
{
	printf("used_memory:\t%d\n", allocator.getUsedMemory());
	printf("num_allocations:\t%d\n", allocator.getNumAllocations());
	printf("size_of_allocator: %d\n", allocator.getSize());
}

}// namespace allocator