// Allocator.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../../Library//MyTools/UnitTestModules.h"
#include "../Allocators/PointerMath.h"
#include "../Allocators/LinearAllocator.h"
#include "../Allocators/StackAllocator.h"
#include "../Allocators/FreeListAllocator.h"
#include "../Allocators/MemoryTracer.h"
#include "../../Library/MyTools/Cleaner.h"
#include "../../Library/MyTools/RandomTool.h"

#pragma comment(lib, "Allocators.lib")

DECLARE_TEST_UNITS;

#define AddrEQ(u32, addr2) (reinterpret_cast<void*>(u32) == addr2)

// this vector store the randoem number between 1`gMaxRandomSize,
// this will be generate in the GetReady().
static const size_t gRandomSeed = 1;
static const size_t gMaxRandomSize = 100;
static std::vector<size_t> gRandomSizes;

// this is used to measure the last usage of the allocator,
// in many tests, we will allocate the memory until it return nullptr,
// but we should ensure that they have really used out the memory,
// with the last buffer size(which make the allocator return mullptr)
// plus current used memories size and the briefAdjustment, 
// the result should greater that the allocator.size.
static const size_t acceptRangeToFullUseOfMemory = 32;

static const size_t gLoopTime = 20;

static const size_t gLinearSize = 256;

static const size_t gStackAllocatorSize = 512;

static const size_t gFreeListAllocatorSize = 2048;

// this marco to declare the linear allocator to use, 
// and will clear the allocator when it's out of current
// life range.
#define DECLARE_LINEAR_ALLOCATOR	\
	void * buffer = malloc(gLinearSize);\
	allocator::LinearAllocator linearAllocator(gLinearSize, buffer);\
	Cleaner clearLinearAlcTrashes([&]() {linearAllocator.clear(); free(buffer);})
#define LinearTarget linearAllocator
// Linear Allocator help marco end


// this marco to declare the linear allocator to use, 
// and will clear the allocator when it's out of current
// life range.
#define DECLARE_STACK_ALLOCATOR	\
	void * buffer = malloc(gStackAllocatorSize);\
	allocator::StackAllocator stackAllocator(gStackAllocatorSize, buffer);\
	Cleaner clearStackAlcTrashes([&]() { free(buffer);})
// Stack Allocator help marco end

#define DECLARE_FREELIST_ALLOCATOR\
	void * buffer = malloc(gFreeListAllocatorSize);\
	allocator::FreeListAllocator freeListAllocator(gFreeListAllocatorSize, buffer);\
	Cleaner clearFreeListAlcTrashes([&](){free(buffer);})

struct TestStruct
{
	int _a;
	short _b;
	long _c;
	TestStruct(int a, short b, long c)
		:_a(a), _b(b), _c(c) {}
	TestStruct():TestStruct(1, 2, 3){}

	inline int report()
	{
		return NOT_EQ(_a, 1)
			+ NOT_EQ(_b, 2)
			+ NOT_EQ(_c, 3);
	}
};

class TestClass
{
public:
	TestClass() : str("tesing string~!@#$%^&*()"), number(123456) {}
	std::string str;
	int number;

	inline int report()
	{
		return NOT_EQ(str, "tesing string~!@#$%^&*()") + NOT_EQ(number, 123456);
	}
};

void TestUnit::GetReady()
{

}

void TestUnit::AfterTest()
{
	
}

void TestUnit::AddTestUnit()
{
	TEST_UNIT_START("first unit startup")
		int error = 0;
		return error == 0;
	TEST_UNIT_END;

	TEST_UNIT_START("test alignForward")
		void* addr1 = reinterpret_cast<void *>(0x00000001);
		void* addr2 = reinterpret_cast<void *>(0x1234567a);
		
		int error = 0;

		error += false == AddrEQ(0x00000002, PointerMath::alignForward(addr1, 2));
		error += false == AddrEQ(0x00000004, PointerMath::alignForward(addr1, 4));
		error += false == AddrEQ(0x00000008, PointerMath::alignForward(addr1, 8));
		error += false == AddrEQ(0x00000010, PointerMath::alignForward(addr1, 16));
		error += false == AddrEQ(0x00000020, PointerMath::alignForward(addr1, 32));
		error += false == AddrEQ(0x1234567c, PointerMath::alignForward(addr2, 4));
		error += false == AddrEQ(0x12345680, PointerMath::alignForward(addr2, 8));
		error += false == AddrEQ(0x12345680, PointerMath::alignForward(addr2, 16));
		error += false == AddrEQ(0x12345680, PointerMath::alignForward(addr2, 32));
		return error  == 0;
	TEST_UNIT_END;

	TEST_UNIT_START("test alignForwardAdjustment")
		void * addr1 = reinterpret_cast<void*>(0x04503302);
		int error = 0;

		error += false == EQ(0, PointerMath::alignForwardAdjustment(addr1, 2));
		error += false == EQ(2, PointerMath::alignForwardAdjustment(addr1, 4));
		error += false == EQ(6, PointerMath::alignForwardAdjustment(addr1, 8));
		error += false == EQ(14, PointerMath::alignForwardAdjustment(addr1, 16));
		error += false == EQ(30, PointerMath::alignForwardAdjustment(addr1, 32));

		return error == 0;
	TEST_UNIT_END;

	TEST_UNIT_START("test alignForwardAdjustment with header")
		void * addr1 = reinterpret_cast<void*>(0x04503302);
		int error = 0;

		error += false == EQ(6, PointerMath::alignForwardAdjustmentWithHeader(addr1, 4, 4 ));
		error += false == EQ(10, PointerMath::alignForwardAdjustmentWithHeader(addr1, 4, 8));
		error += false == EQ(6, PointerMath::alignForwardAdjustmentWithHeader(addr1, 8, 4));
		error += false == EQ(38, PointerMath::alignForwardAdjustmentWithHeader(addr1, 8, 32));
		
		return error == 0;
	TEST_UNIT_END;

	TEST_UNIT_START("test linearAllocatorm once")
		int error = 0;
		DECLARE_LINEAR_ALLOCATOR;

		std::vector<void*> memories;
		std::vector<size_t> randomSizes;
		RandomTool::RandomNumbers<size_t>(2000, &randomSizes, 100, 1);
		void * ptemp = nullptr;

		memories.clear();
		size_t lastRejectSize = 0;
		for (auto & size : randomSizes)
		{
			ptemp = linearAllocator.allocate(size, 1);
			if (ptemp)
			{
				memories.push_back(ptemp);
			}
			else
			{
				lastRejectSize = size;
				break;
			}
		}
		// ensure that there is not much memory left.
		error += 
			(linearAllocator.getSize() - (lastRejectSize + linearAllocator.getUsedMemory()))
			< acceptRangeToFullUseOfMemory;
		
		// deallocate use clear
		linearAllocator.clear();

		error += 0 != linearAllocator.getUsedMemory();
		error += 0 != linearAllocator.getNumAllocations();
		return error == 0;
	TEST_UNIT_END;

	TEST_UNIT_START("loop allocate and deallocate with linearAllocator")
		int error = 0;
		DECLARE_LINEAR_ALLOCATOR;

		std::vector<void*> memories;
		std::vector<size_t> usedMemoryLogger;
		// this is used to log the first loop of the operation,
		// afther this, any loop should be the same as the first one,
		// if not, there is an error.
		std::vector<size_t> randomSizes;
		RandomTool::RandomNumbers<size_t>(2000, &randomSizes, 100, 1);
		void * ptemp = nullptr;
		size_t lastRejectSize = 0;

		for (int i = 0; i < gLoopTime; ++i)
		{
			memories.clear();
			for (auto & size : randomSizes)
			{
				ptemp = linearAllocator.allocate(size, 1);
				if (ptemp)
				{
					memories.push_back(ptemp);

					// log only the first loop
					if (i == 0)
					{
						usedMemoryLogger.push_back(linearAllocator.getUsedMemory());
					}
					else// any other loop should behave the same as the first one
					{
						lastRejectSize = size;
						error += 
							usedMemoryLogger[linearAllocator.getNumAllocations() - 1]
							!= linearAllocator.getUsedMemory();
					}
				}
				else
				{
					// the allocation size should also the same as the first one,
					// here we don't distinguish first loop and after
					lastRejectSize = size;
					error += usedMemoryLogger.size() != (linearAllocator.getNumAllocations());
					break;
				}
			}// for size in radomSizes

			 // ensure that there is not much memory left.
			error +=
				(linearAllocator.getSize() - (lastRejectSize + linearAllocator.getUsedMemory()))
				< acceptRangeToFullUseOfMemory;

			// deallocate
			linearAllocator.clear();
			error += linearAllocator.getUsedMemory() != 0;
			error += linearAllocator.getNumAllocations() != 0;
		}// for gLoopTime

		return error == 0;
	TEST_UNIT_END;

	TEST_UNIT_START("can StackAllocator usually work?")
		int error = 0;
		DECLARE_STACK_ALLOCATOR;

		std::vector<void*> memories;
		std::vector<size_t> randomSizes;
		RandomTool::RandomNumbers<size_t>(2000, &randomSizes, 100, 1);
		void * ptemp = nullptr;
		size_t lastRejectSize = 0;

		memories.clear();
		for (auto & size : randomSizes)
		{
			ptemp = stackAllocator.allocate(size, 1);
			if (ptemp)
			{
				memories.push_back(ptemp);
			}
			else
			{
				lastRejectSize = size;
				break;
			}
		}
		// deallocate them in the reverse order
		for (int i = memories.size() - 1; i >= 0; --i)
		{
			stackAllocator.deallocate(memories[i]);
		}

		error += 0 != stackAllocator.getUsedMemory();
		error += 0 != stackAllocator.getNumAllocations();
		return error == 0;
	TEST_UNIT_END;

	TEST_UNIT_START("for stack allocator loop allocate and free")
		int error = 0;
		DECLARE_STACK_ALLOCATOR;

		std::vector<void*> memories;
		std::vector<size_t> usedMemoryLogger;
		// this is used to log the first loop of the operation,
		// afther this, any loop should be the same as the first one,
		// if not, there is an error.
		std::vector<size_t> randomSizes;
		RandomTool::RandomNumbers<size_t>(2000, &randomSizes, 100, 1);
		void * ptemp = nullptr;
		size_t lastRejectSize = 0;

		for (int i = 0; i < gLoopTime; ++i)
		{
			memories.clear();
			for (auto & size : randomSizes)
			{
				ptemp = stackAllocator.allocate(size, 1);
				if (ptemp)
				{
					memories.push_back(ptemp);

					// log only the first loop
					if (i == 0)
					{
						usedMemoryLogger.push_back(stackAllocator.getUsedMemory());
					}
					else// any other loop should behave the same as the first one
					{
						lastRejectSize = size;
						error += 
							usedMemoryLogger[stackAllocator.getNumAllocations() - 1]
							!= stackAllocator.getUsedMemory();
					}
				}
				else
				{
					// the allocation size should also the same as the first one,
					// here we don't distinguish first loop and after
					error += usedMemoryLogger.size() != (stackAllocator.getNumAllocations());
					break;
				}
			}

			// deallocate them in the reverse order
			for (int i = memories.size() - 1; i >= 0; --i)
			{
				stackAllocator.deallocate(memories[i]);
			}
			error += stackAllocator.getUsedMemory() != 0;
			error += stackAllocator.getNumAllocations() != 0;
		}

		return error == 0;
	TEST_UNIT_END;

	TEST_UNIT_START("normal use of FreeListAllocator")
		int error = 0;
		DECLARE_FREELIST_ALLOCATOR;
		int * shouldBe3 = allocator::AllocateNew<int>(freeListAllocator);
		*shouldBe3 = 3;

		const size_t bufferSize = 50;
		void* pAllocatedBuffer = freeListAllocator.allocate(bufferSize, 1);

		error += pAllocatedBuffer == nullptr;
		error += *shouldBe3 != 3;

		freeListAllocator.deallocate(shouldBe3);
		freeListAllocator.deallocate(pAllocatedBuffer);
		return error == 0;
	TEST_UNIT_END;

	TEST_UNIT_START("FreeListAllocator randomly allocate and deallocate")
		int error = 0;
		DECLARE_FREELIST_ALLOCATOR;

		std::vector<void*> memories;
		std::vector<size_t> randomSizes;
		std::vector<size_t> randomIndices;
		RandomTool::RandomNumbers<size_t>(2000, &randomSizes, 100, 1);
		void * ptemp = nullptr;
		size_t lastRejectSize = 0;

		// allocate
		for (auto & size : randomSizes)
		{
			ptemp = freeListAllocator.allocate(size, 1);
			if (ptemp)
			{
				//DEBUG_MESSAGE("allocated with size: %d\n", size);
				//DEBUG_MESSAGE("allocatore state: total:%d\tused:%d\n", freeListAllocator.getSize(), freeListAllocator.getUsedMemory());
				memories.push_back(ptemp);
			}
			else
			{
				lastRejectSize = size;
				break;
			}
		}

		// deallocate
		RandomTool::RandomSequence<size_t>(memories.size(), &randomIndices);

		for (auto & index : randomIndices)
		{
			freeListAllocator.deallocate(memories[index]);
			memories[index] = nullptr;
		}

		error += freeListAllocator.getUsedMemory() != 0;
		error += freeListAllocator.getNumAllocations() != 0;

		return error == 0;
	TEST_UNIT_END;

	TEST_UNIT_START("loop the allocation and free with FreeListAllocator")
		int error = 0;
		DECLARE_FREELIST_ALLOCATOR;

		std::vector<void*> memories;
		std::vector<size_t> usedMemoryLogger;
		// this is used to log the first loop of the operation,
		// afther this, any loop should be the same as the first one,
		// if not, there is an error.
		std::vector<size_t> randomSizes;
		std::vector<size_t> randomIndices;
		RandomTool::RandomNumbers<size_t>(2000, &randomSizes, 100, 1);
		void * ptemp = nullptr;
		size_t lastRejectSize = 0;

		for (int i = 0; i < gLoopTime; ++i)
		{
			//DEBUG_MESSAGE("loop freeListAllocator round: %d\n", i);
			memories.clear();
			for (auto & size : randomSizes)
			{
				ptemp = freeListAllocator.allocate(size, 1);
				if (ptemp)
				{
					//DEBUG_MESSAGE("allocated with size: %d\n", size);
					//DEBUG_MESSAGE("allocatore state: total:%d\tused:%d\n", freeListAllocator.getSize(), freeListAllocator.getUsedMemory());
					memories.push_back(ptemp);

					// log only the first loop
					if (i == 0)
					{
						usedMemoryLogger.push_back(freeListAllocator.getUsedMemory());
					}
					else// any other loop should behave the same as the first one
					{
						lastRejectSize = size;
						error += 
							usedMemoryLogger[freeListAllocator.getNumAllocations() - 1] 
							!= freeListAllocator.getUsedMemory();
					}
				}
				else
				{
					// the allocation size should also the same as the first one,
					// here we don't distinguish first loop and after
					error += usedMemoryLogger.size() != (freeListAllocator.getNumAllocations());
					break;
				}
			}
			// deallocate
			RandomTool::RandomSequence<size_t>(memories.size(), &randomIndices);

			for (auto & index : randomIndices)
			{
				freeListAllocator.deallocate(memories[index]);
				memories[index] = nullptr;
			}
			error += freeListAllocator.getUsedMemory() != 0;
			error += freeListAllocator.getNumAllocations() != 0;
		}

		return error == 0;
	TEST_UNIT_END;

	/*TEST_UNIT_START("test the random sequence generator, this should always failed")
		std::vector<size_t> sequence;
		RandomTool::RandomSequence<size_t>(10, &sequence, 6);
		for (auto & t : sequence)
		{
			DEBUG_MESSAGE("rand sequence: %d\n", t);
		}
		return false;
	TEST_UNIT_END;

	TEST_UNIT_START("test random number generator, this should always failed")
		std::vector<size_t> sequence;
		RandomTool::RandomNumbers<size_t>(30, &sequence, 15);
		for (auto & t : sequence)
		{
			DEBUG_MESSAGE("rand number: %d\n", t);
		}
		return false;
	TEST_UNIT_END;*/
}


int main()
{
	TestUnit::testMain();
    return 0;
}

