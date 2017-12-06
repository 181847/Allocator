#pragma once
#include <stdio.h>
#include "../../Library/MyTools/UsefulDataType.h"
#include "../../Library/MyTools/MetaTools.h"

namespace PointerMath
{



inline void printAddress(void * address)
{
	printf("%08p\n", address);
}
inline void printAddress(int address)
{
	printf("%08x\n", address);
}

template<u32 addr, typename T>
struct ALIGN_FORWARD_CLASS
{
	
public:
	enum {
		ret = (addr + (alignof(T)- 1))
			& ~(alignof(T)- 1)
	};
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
	enum { ret = sizeof(headType) / sizeof(T) + reType::ret };
};// ALIGN_ARR_HEADER

inline void* alignForward(void * addr, u8 align)
{
	return 
		reinterpret_cast<void*>(
			(reinterpret_cast<u32>(addr) + static_cast<u32>(align - 1)) 
		& static_cast<u32>(~(align - 1)));
}

inline u8 alignForwardAdjustment(void * addr, u8 alignment)
{
	u8 adjust = alignment - reinterpret_cast<u8>(addr) & (alignment - 1);
	// if adjust is the alignment, return 0.
	return (adjust != alignment) * adjust;
}

inline u8 alignForwardAdjustmentWithHeader(void * addr, u8 aligment, u8 headSize)
{

	u8 addjustment = alignForwardAdjustment(addr, aligment);
	u8 needSpace = headSize;
	
	// if the addjustment greater than the headSize
	// we should increase the addjustment
	if (addjustment < needSpace)
	{
		// how many left should we need besides the previous addjustment
		needSpace -= addjustment;
		addjustment += aligment * (
			(needSpace + 
			// if needSpace mod aligment > 0 mean we need additional one
			// aligment to to feed the needSpace.
			(needSpace % aligment != 0)*(aligment)) / aligment);
		// after the needSpace if still some outof the aligment
		/*if (needSpace % aligment != 0)
		{
			addjustment += aligment;
		}*/
	}
	return addjustment;
}

// offset the address up
inline void * add(void * addr, size_t offset)
{
	return reinterpret_cast<void*>(reinterpret_cast<DWORD>(addr) +offset);
}

template<typename T>
inline T* add(T * addr, size_t offset)
{
	return reinterpret_cast<T*>(reinterpret_cast<void*>(addr) + offset));
}

// substruct the address by some offset
inline void * substract(void * addr, size_t offset)
{
	return reinterpret_cast<void*>(reinterpret_cast<DWORD>(addr) - offset);
}

// get the distanc of two address
inline size_t substract(void * higherAddr, void * lowerAddr)
{
	return reinterpret_cast<size_t>(higherAddr) -
		reinterpret_cast<size_t>(lowerAddr);
}

}// allocator
