#pragma once
#include <stdio.h>
#include "../../Library/MyTools/UsefulDataType.h"

namespace PointerMath
{



inline void printAddress(void * address)
{
	printf("%08x\n", address);
}
inline void printAddress(int address)
{
	printf("%08x\n", address);
}

inline void* alignForward(void * addr, u8 align)
{
	//int a = reinterpret_cast<int>(addr);
	//return (void*)((a + (align - 1)) & ~(align - 1));
	return reinterpret_cast<void*>((reinterpret_cast<u32>(addr) + static_cast<u32>(align - 1)) 
		& static_cast<u32>(~(align - 1)));
}

inline u8 alignForwardAdjustment(void * addr, u8 alignment)
{
	u8 adjust = alignment - reinterpret_cast<u8>(addr) & (alignment - 1);
	// if adjust is the alignment, return 0.
	if (adjust == alignment)
		return 0;
	return adjust;
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
		addjustment += aligment * (needSpace / aligment);
		// after the needSpace if still some outof the aligment
		if (needSpace % aligment != 0)
		{
			addjustment += aligment;
		}
	}
	return addjustment;
}

inline void * add(void * addr, size_t offset)
{
	return reinterpret_cast<void*>(reinterpret_cast<DWORD>(addr) +offset);
}

inline void * substract(void * addr, size_t offset)
{
	return reinterpret_cast<void*>(reinterpret_cast<DWORD>(addr) - offset);
}

}// allocator
