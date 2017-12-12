// Allocator.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../../Library//MyTools/UnitTestModules.h"
#include "../Allocators/PointerMath.h"
#include "../Allocators/LinearAllocator.h"
#include "../Allocators/StackAllocator.h"
#include "../Allocators/FreeListAllocator.h"
#include "../Allocators/PoolAllocator.h"
#include "../Allocators/MemoryTracer.h"
#include "../../Library/MyTools/Cleaner.h"
#include "../../Library/MyTools/RandomTool.h"

#pragma comment(lib, "Allocators.lib")

DECLARE_TEST_UNITS;

#define AddrEQ(u32, addr2) (reinterpret_cast<void*>(u32) == addr2)



static const size_t acceptRangeToFullUseOfMemory = 32;
inline bool checkRestMemory(
	allocator::Allocator& alctor, 
	size_t lastRejectSize, size_t acceptableRange = acceptRangeToFullUseOfMemory)
{
	size_t leftMemory = alctor.getSize() - alctor.getUsedMemory();
	// the left Memoryt shouldn't be greater than the 
	// sum of lastRejectSize and accptablRange.
	// if it does, means there is a lot of memory not used.
	return leftMemory > lastRejectSize + acceptableRange;
}


enum DeallocateOrder{ Random, BackToFront, Unordered};

inline int randomAllocaAndDeallocaTest(
	allocator::Allocator& alc, int loopTime = 20,
	// sizeCount how many 'size' will be generator per batch, maybe some is not used.
	// maxSize: the max random alloca size
	// minSize: the minmum random size
	// align:   static allign through the every allocate.
	int sizeCount = 2000, int maxSize = 100, int minSize = 1, u8 align = 1,

	// randomPerBatch: did each batch generate a new randoma size serials?
	// randSeed:       global random seed for every random operattion inside.
	bool randomPerBatch = false, DeallocateOrder deallocOrder = DeallocateOrder::Random, int randSeed = 1)
{
	int error = 0;
	std::vector<void*> memories;
	std::vector<size_t> usedMemoryLogger;
	// this is used to log the first loop of the operation,
	// afther this, any loop should be the same as the first one,
	// if not, there is an error.
	std::vector<size_t> randomSizes;
	std::vector<size_t> randomIndices;
	RandomTool::RandomNumbers<size_t>(sizeCount, &randomSizes, maxSize, minSize, randSeed);
	void * ptemp = nullptr;
	size_t lastRejectSize = 0;

	for (int i = 0; i < loopTime; ++i)
	{
		if (randomPerBatch)
		{
			RandomTool::RandomNumbers<size_t>(sizeCount, &randomSizes, maxSize, minSize, i + randSeed);
		}
		//DEBUG_MESSAGE("loop freeListAllocator round: %d\n", i);
		memories.clear();
		for (auto & size : randomSizes)
		{
			ptemp = alc.allocate(size, align);
			if (ptemp)
			{
				//DEBUG_MESSAGE("allocated with size: %d\n", size);
				//DEBUG_MESSAGE("allocatore state: total:%d\tused:%d\n", freeListAllocator.getSize(), freeListAllocator.getUsedMemory());
				memories.push_back(ptemp);

				// if every batch is the same allocation sizes,
				// check for each batch, that every allocate should the 
				// same as the first batch.
				if ( ! randomPerBatch)
				{
					// log only the first batch
					if (i == 0)
					{
						usedMemoryLogger.push_back(alc.getUsedMemory());
					}
					else// any other batch should behave the same as the first one
					{
						// the allocation size should also the same as the first one,
						error += usedMemoryLogger[alc.getNumAllocations() - 1] != alc.getUsedMemory();
					}
				}
				
			}
			else
			{
				lastRejectSize = size;
				break;
			}
		}
		// check rest memory is not much ,
		// because the memory should have been used to the max.
		error += checkRestMemory(alc, lastRejectSize);

		// deallocate
		switch (deallocOrder)
		{
		case DeallocateOrder::BackToFront:
			// deallocate them in the reverse order
			for (int i = memories.size() - 1; i >= 0; --i)
			{
				alc.deallocate(memories[i]);
			}
			break;

		case DeallocateOrder::Random:
			if (randomPerBatch) // random indices per batch
				RandomTool::RandomSequence<size_t>(memories.size(), &randomIndices, i + randSeed);
			else				// same indices per batch with differ size.
				RandomTool::RandomSequence<size_t>(memories.size(), &randomIndices, randSeed);

			for (auto & index : randomIndices)
			{
				alc.deallocate(memories[index]);
				memories[index] = nullptr;
			}
			break;
		}

		// ensure all the memory have been freed.
		error += alc.getUsedMemory() != 0;
		error += alc.getNumAllocations() != 0;
	}

	return error;
}
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

static const size_t gLoopTime = 20;

static const size_t gLinearSize = 256;

static const size_t gStackAllocatorSize = 512;

static const size_t gFreeListAllocatorSize = 2048;

static const size_t gPoolAllocatorSize = 2048;

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

#define DECLARE_POOL_ALLOCATOR(type)\
	void * buffer = malloc(gPoolAllocatorSize);\
	allocator::PoolAllocator poolAllocator(sizeof(type), alignof(type), gPoolAllocatorSize, buffer);\
	Cleaner clearPoolAlcTrashes([&](){free(buffer);})

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
		error += checkRestMemory(linearAllocator, lastRejectSize);
		
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
			error += checkRestMemory(linearAllocator, lastRejectSize);

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

		// check rest memory is not much ,
		// because the memory should have been used to the max.
		error += checkRestMemory(stackAllocator, lastRejectSize);

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
		error += randomAllocaAndDeallocaTest(
			stackAllocator, 20, 2000, 100, 1, 1,
			// each batch is the same
			false, DeallocateOrder::BackToFront, 1);
		error += randomAllocaAndDeallocaTest(
			stackAllocator, 20, 2000, 100, 1, 1,
			// each batch is the same
			true, DeallocateOrder::BackToFront, 1);

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

	TEST_UNIT_START("loop the allocation and free with FreeListAllocator")
		int error = 0;
		DECLARE_FREELIST_ALLOCATOR;
		error += randomAllocaAndDeallocaTest(
			freeListAllocator, 20, 2000, 100, 1, 1, 
			// each batch is the same
			false, DeallocateOrder::Random, 1);
		
		error += randomAllocaAndDeallocaTest(
			freeListAllocator, 20, 2000, 100, 1, 1, 
			// random per batch
			true, DeallocateOrder::Random, 1);

		return error == 0;
	TEST_UNIT_END;

	TEST_UNIT_START("test allocator.AllocateNew with linearAllocator")
		DECLARE_LINEAR_ALLOCATOR;
		int error = 0;
		const int arrLength = 5;
		TestClass * ptc = allocator::AllocateNew<TestClass>(linearAllocator);

		error += ptc->report();

		linearAllocator.clear();
		return error == 0;
	TEST_UNIT_END;

	TEST_UNIT_START("test allocator.AllocateArray with linearAllocator")
		DECLARE_LINEAR_ALLOCATOR;
		int error = 0;
		const int arrLength = 5;
		TestClass * ptc_arr = allocator::AllocateArray<TestClass>(linearAllocator, arrLength);
		for (int i = 0; i < arrLength; ++i)
		{
			error += ptc_arr[i].report();
		}
		linearAllocator.clear();
		return error == 0;
	TEST_UNIT_END;

	TEST_UNIT_START("test allocator.AllocateNew with stackAllocator")
		DECLARE_STACK_ALLOCATOR;
		int error = 0;
		const int arrLength = 5;
		TestClass * ptc = allocator::AllocateNew<TestClass>(stackAllocator);

		error += ptc->report();

		allocator::Deallocate(stackAllocator, ptc);
		return error == 0;
	TEST_UNIT_END;

	TEST_UNIT_START("test allocator.AllocateArray with stackAllocator")
		DECLARE_STACK_ALLOCATOR;
		int error = 0;
		const int arrLength = 5;
		TestClass * ptc_arr = allocator::AllocateArray<TestClass>(stackAllocator, arrLength);
		for (int i = 0; i < arrLength; ++i)
		{
			error += ptc_arr[i].report();
		}
		allocator::DeallocateArray(stackAllocator, ptc_arr);
		return error == 0;
	TEST_UNIT_END;

	TEST_UNIT_START("test allocator.AllocateNew with freeListAllocator")
		DECLARE_FREELIST_ALLOCATOR;
		int error = 0;
		const int arrLength = 5;
		TestClass * ptc = allocator::AllocateNew<TestClass>(freeListAllocator);

		error += ptc->report();

		allocator::Deallocate(freeListAllocator, ptc);
		return error == 0;
	TEST_UNIT_END;

	TEST_UNIT_START("test allocator.AllocateArray with freeListAllocator")
		DECLARE_FREELIST_ALLOCATOR;
		int error = 0;
		const int arrLength = 5;
		TestClass * ptc_arr = allocator::AllocateArray<TestClass>(freeListAllocator, arrLength);
		for (int i = 0; i < arrLength; ++i)
		{
			error += ptc_arr[i].report();
		}
		allocator::DeallocateArray(freeListAllocator, ptc_arr);
		return error == 0;
	TEST_UNIT_END;

	

	TEST_UNIT_START("test pool allocator")
		int error = 0;

		DECLARE_POOL_ALLOCATOR(TestStruct);

		error += randomAllocaAndDeallocaTest(
			poolAllocator, 20, 2000, sizeof(TestStruct), sizeof(TestStruct), alignof(TestStruct));
		
		return error == 0;
	TEST_UNIT_END;
}


int main()
{
	TestUnit::testMain();
    return 0;
}

