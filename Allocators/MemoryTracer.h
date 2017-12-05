#pragma once
#include "Allocator.h"
#include "LinearAllocator.h"


namespace debug
{

namespace debugAllocator
{

// base memory tracer
class MemoryTracer
{
public:
	MemoryTracer(allocator::Allocator& alctr)
		:usedMemory(0), basePointer(reinterpret_cast<u32>(alctr.getStart()))
		, targetAllocator(alctr), numAllocations(0)
	{
		ASSERT(alctr.getUsedMemory() == 0 && "MemoryTracer ERROR, the allocator shouldn't be used before trace");
	}
	DELETE_COPY_CONSTRUCTOR(MemoryTracer)
	~MemoryTracer()
	{}

	// if you call next two function , you should call Tick()
	// to trace the allocations count.
	template<typename T>
	void NewArray(size_t length) = 0;
	template<typename T>
	void New() = 0;

	// report will report how many dis match curred 
	// during the this report,
	// it will check the usedMemory ans the allocations should be same.
	// it should be 0(no error)/ 1/ 2(one or two error happended).
	inline int report()
	{
		return 
				usedMemory != targetAllocator.getUsedMemory()
			+	numAllocations != targetAllocator.getNumAllocations();
	}

	inline void tick()
	{
		++numAllocations;
	}

public:
	u32 basePointer;
	size_t usedMemory;
	allocator::Allocator& targetAllocator;
	size_t numAllocations;
};// memory tracer

class LinearMemoryTracer
	:public MemoryTracer
{

public:
	LinearMemoryTracer(allocator::Allocator& alctr)
		:MemoryTracer(alctr)
	{}
	DELETE_COPY_CONSTRUCTOR(LinearMemoryTracer)


	template<typename T>
	inline void NewArray(size_t length)
	{
		tick();
		usedMemory += (PointerMath::ALIGN_ARR_HEADER<T>::ret + length) * sizeof(T)
			+ PointerMath::alignForwardAdjustment(
				reinterpret_cast<void*>(basePointer + usedMemory), alignof(T));
	}

	template<typename T>
	inline void New()
	{
		tick();
		usedMemory += PointerMath::alignForwardAdjustment(
			(void*)basePointer, alignof(T))
			+ sizeof(T);
	}
	
};// LinearMemory Tracer

}// allocator
}// debug namespace