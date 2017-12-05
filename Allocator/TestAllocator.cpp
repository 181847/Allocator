// Allocator.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../../Library//MyTools/UnitTestModules.h"
#include "../Allocators/PointerMath.h"
#include "../Allocators/LinearAllocator.h"
#include "../Allocators/StackAllocator.h"
#include "../Allocators/MemoryTracer.h"
#include "../../Library/MyTools/Cleaner.h"

#pragma comment(lib, "Allocators.lib")

DECLARE_TEST_UNITS;

#define AddrEQ(u32, addr2) (reinterpret_cast<void*>(u32) == addr2)

static const size_t gLinearSize = 256;

static const size_t gStackAllocatorSize = 512;

// this marco to declare the linear allocator to use, 
// and will clear the allocator when it's out of current
// life range.
#define DECLARE_LINER_ALLOCATOR	\
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

	TEST_UNIT_START("test use out of the gLinearTarget with char")
		DECLARE_LINER_ALLOCATOR;
		void * pstart = LinearTarget.getStart();
		
		for (int i = 0; i < gLinearSize; ++i)
		{
			allocator::AllocateNew<char>(LinearTarget);
		}

		int error = 0;

		error += nullptr != LinearTarget.allocate(1, 1);
		
		return LinearTarget.getUsedMemory() == gLinearSize
			&& error == 0;
	TEST_UNIT_END;

	TEST_UNIT_START("use out the LinearTarget with array")
		int error = 0;
		DECLARE_LINER_ALLOCATOR;

		void * pstart = LinearTarget.getStart();

		const size_t headAdjustment = PointerMath::alignForwardAdjustment(pstart, alignof(u16))
			+ PointerMath::ALIGN_ARR_HEADER<u16>::ret;
		const size_t restMemory = gLinearSize - headAdjustment * sizeof(u16);

		size_t maxLength = restMemory / sizeof(u16);
		u16 * pArr = allocator::AllocateArray<u16>(LinearTarget, maxLength);

		// this allocation should be success,after this allocation,
		// any allocation will return nullptr.
		error += pArr == nullptr;

		error += nullptr != allocator::AllocateNew<u8>(LinearTarget);

		return error == 0;
	TEST_UNIT_END;

	TEST_UNIT_START("test clear LinearTarget")
		int error = 0;
		DECLARE_LINER_ALLOCATOR;

		void * pstart = LinearTarget.getStart();

		// first allocate all the memory
		for (int i = 0; i < gLinearSize; ++i)
		{
			allocator::AllocateNew<char>(LinearTarget);
		}
		// ensure that we cannot allocate any byte from the LinearTarget.
		error += nullptr != LinearTarget.allocate(1, 1);
	
		// clear the LinearTarget
		LinearTarget.clear();
		
		// mallocate all the memory with a array of u16.
		const size_t	headAdjustment = PointerMath::alignForwardAdjustment(pstart, alignof(u16))
											+ PointerMath::ALIGN_ARR_HEADER<u16>::ret;
		const size_t	restMemory = gLinearSize - headAdjustment * sizeof(u16);
		size_t			maxLength = restMemory / sizeof(u16);
		u16 *			pArr = allocator::AllocateArray<u16>(LinearTarget, maxLength);

		// dose the allocation success?
		error += pArr == nullptr;

		// and we will not get any other byte from the allocator.
		error += nullptr != allocator::AllocateNew<u8>(LinearTarget);
		
		return error == 0;
	TEST_UNIT_END;

	TEST_UNIT_START("test malloc from gLinearTarget")
		DEBUG_MESSAGE("TestStruct size: %d\n", sizeof(TestStruct));
		DEBUG_MESSAGE("TestStruct align: %d\n", alignof(TestStruct));
		DECLARE_LINER_ALLOCATOR;
		debug::debugAllocator::LinearMemoryTracer tracer(LinearTarget);

		int error = 0;

		auto * shouldBeOne = allocator::AllocateNew<int>(LinearTarget);
		tracer.New<int>();
		*shouldBeOne = 1;

		error += tracer.report();

		const size_t arrLength = 5;
		auto * pTestStructArr = allocator::AllocateArray<TestStruct>(LinearTarget, arrLength);
		tracer.NewArray<TestStruct>(arrLength);
		for (int i = 0; i < arrLength; ++i)
		{
			error += NOT_EQ(1, pTestStructArr[i]._a);
			error += NOT_EQ(2, pTestStructArr[i]._b);
			error += NOT_EQ(3, pTestStructArr[i]._c);
		}

		const size_t longArrLength = 5;
		auto plongArr = allocator::AllocateArray<unsigned long>(LinearTarget, longArrLength);
		tracer.NewArray<unsigned long>(longArrLength);

		error += 1 != *shouldBeOne;
		error += tracer.report();
		
		// WARNING this code will cause tracer dismatch the LinearTarget,
		// because we want to test the tracer truly log the state
		// of the allocator.
		// so if corect, afther this operation,
		// the trace should return a number > 0.
		allocator::AllocateNew<int>(LinearTarget);

		// if it return 0,
		// there is some error in the tracer.
		error += tracer.report() == 0;

		//allocator::showAllocator(LinearTarget);
		return error == 0;
	TEST_UNIT_END;

	TEST_UNIT_START("can StackAllocator usually work?")
		DECLARE_STACK_ALLOCATOR;
		int error = 0;
		debug::debugAllocator::StackMemoryTracer tracer(stackAllocator);

		int * pShouldBe2 = allocator::AllocateNew<int>(stackAllocator);
		tracer.New<int>();
		*pShouldBe2 = 2;
		error += tracer.report();

		const int arrLength = 5;
		TestStruct * pTStruct = allocator::AllocateArray<TestStruct>(stackAllocator, arrLength);
		tracer.NewArray<TestStruct>(arrLength);

		for (int i = 0; i < arrLength; ++i)
		{
			error += NOT_EQ(1, pTStruct[i]._a);
			error += NOT_EQ(2, pTStruct[i]._b);
			error += NOT_EQ(3, pTStruct[i]._c);
		}
		error += 2 != *pShouldBe2;

		error += tracer.report();

		// to successfully end the use of stackAllocator, 
		// we must free the memory,
		// or the assert will be triggered in the deconstructor
		// of Allocator, which dosen't allowed to free the allocator
		// when still some object is on the memory.
		allocator::DeallocateArray(stackAllocator, pTStruct);
		allocator::Deallocate(stackAllocator, pShouldBe2);
		return error == 0;
	TEST_UNIT_END;

	TEST_UNIT_START("for stack allocator loop allocate and free (200000 times)")
		int error = 0;
		DECLARE_STACK_ALLOCATOR;
		const size_t loopTimes = 200000;
		for (int i = 0; i < loopTimes; ++i)
		{

			int * pShouldBe2 = allocator::AllocateNew<int>(stackAllocator);
			*pShouldBe2 = 2;

			const int arrLength = 5;
			TestStruct * pTStruct = allocator::AllocateArray<TestStruct>(stackAllocator, arrLength);
			TestClass * pTClass = allocator::AllocateArray<TestClass>(stackAllocator, arrLength);

			for (int i = 0; i < arrLength; ++i)
			{
				error += pTStruct[i].report();
				error += pTClass[i].report();
			}
			error += 2 != *pShouldBe2;

			// to successfully end the use of stackAllocator, 
			// we must free the memory,
			// or the assert will be triggered in the deconstructor
			// of Allocator, which dosen't allowed to free the allocator
			// when still some object is on the memory.
			allocator::DeallocateArray(stackAllocator, pTClass);
			allocator::DeallocateArray(stackAllocator, pTStruct);
			allocator::Deallocate(stackAllocator, pShouldBe2);
		}
		error += stackAllocator.getUsedMemory() != 0;

		return error == 0;
	TEST_UNIT_END;
}


int main()
{
	TestUnit::testMain();
    return 0;
}

