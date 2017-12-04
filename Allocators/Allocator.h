#pragma once
#include "../../Library/MyTools/UsefulDataType.h"
#include "../../Library/MyTools/MyAssert.h"
#include "../../Library/MyTools/MyTools.h"
#include "../../Library/MyTools/DLLTools.h"
#include "PointerMath.h"
#include "MetaTools.h"

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

template<size_t number>
struct NUMBER_CONTAINER 
{
public:
	enum { ret = number };
};

// This struct is used to calculate the at least how many T
// should be used to contain a headType.
// if T is 'unsigned char'(1 Byte), headType is 'size_t'(4 Byte)
// the ret should be 4,
// if T is 'unsigned long'(8 Byte), headType is 'size_t'(4 Byte)
// the ret should be only 1, even another 4 Byte is not used.
template<typename T, typename headType = size_t>
struct ALIGN_ARR_HEADER
{
public:
	typedef typename
		IF__<
		sizeof(headType) % sizeof(T) == 0,
		NUMBER_CONTAINER<0>,
		NUMBER_CONTAINER<1>>::reType
	reType;
	enum {ret = sizeof(headType) / sizeof(T) + reType::ret};
};// ALIGN_ARR_HEADER

template<typename T>
T* AllocateArray(Allocator& allocator, size_t length)
{
	size_t headSize = ALIGN_ARR_HEADER<T>::reType::ret;

	// get the array address,
	// we will jump front head length;
	T* p = headSize + reinterpret_cast<T*>(allocator.allocate(sizeof(T) * (length + headSize), alignof(T)));
	// log length
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
	size_t length = *(reinterpret_cast<size_t*>(parray) - 1);

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