// Allocator.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../../Library//MyTools/UnitTestModules.h"
#include "../Allocators/PointerMath.h"
#include "../Allocators/LinearAllocator.h"
#include "../Allocators/MemoryTracer.h"
#include "../../Library/MyTools/Cleaner.h"

#pragma comment(lib, "Allocators.lib")

DECLARE_TEST_UNITS;

#define AddrEQ(u32, addr2) (reinterpret_cast<void*>(u32) == addr2)

static const size_t gLinearSize = 256;
static allocator::LinearAllocator gLinearAllocator(
	gLinearSize,
	malloc(gLinearSize));

struct TestStruct
{
	int _a;
	short _b;
	long _c;
	TestStruct(int a, short b, long c)
		:_a(a), _b(b), _c(c) {}
	TestStruct()
		:_a(1), _b(2), _c(3) {}
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
		error += gLinearSize != gLinearAllocator.getSize();
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

	TEST_UNIT_START("test use out of the gLinearAllocator with char")
		allocator::LinearAllocator linearAllocator(gLinearSize, malloc(gLinearSize));
		Cleaner clearLinearAlc([&](){linearAllocator.clear();});
		void * pstart = linearAllocator.getStart();
		
		for (int i = 0; i < gLinearSize; ++i)
		{
			allocator::AllocateNew<char>(linearAllocator);
		}

		int error = 0;

		error += nullptr != linearAllocator.allocate(1, 1);
		
		return linearAllocator.getUsedMemory() == gLinearSize
			&& error == 0;
	TEST_UNIT_END;

	TEST_UNIT_START("use out the LinearAllocator with array")
		int error = 0;
		allocator::LinearAllocator linearAllocator(gLinearSize, malloc(gLinearSize));
		Cleaner clearLinearAlc([&]() {linearAllocator.clear(); });
		void * pstart = linearAllocator.getStart();

		const size_t headAdjustment = PointerMath::alignForwardAdjustment(pstart, alignof(u16))
			+ PointerMath::ALIGN_ARR_HEADER<u16>::ret;
		const size_t restMemory = gLinearSize - headAdjustment * sizeof(u16);

		size_t maxLength = restMemory / sizeof(u16);
		u16 * pArr = allocator::AllocateArray<u16>(linearAllocator, maxLength);

		// this allocation should be success,after this allocation,
		// any allocation will return nullptr.
		error += pArr == nullptr;

		error += nullptr != allocator::AllocateNew<u8>(linearAllocator);

		return error == 0;
	TEST_UNIT_END;

	TEST_UNIT_START("test clear LinearAllocator")
		int error = 0;
		allocator::LinearAllocator linearAllocator(gLinearSize, malloc(gLinearSize));
		Cleaner clearLinearAlc([&]() {linearAllocator.clear(); });
		void * pstart = linearAllocator.getStart();

		// first allocate all the memory
		for (int i = 0; i < gLinearSize; ++i)
		{
			allocator::AllocateNew<char>(linearAllocator);
		}
		// ensure that we cannot allocate any byte from the linearAllocator.
		error += nullptr != linearAllocator.allocate(1, 1);
	
		// clear the linearAllocator
		linearAllocator.clear();
		
		// mallocate all the memory with a array of u16.
		const size_t	headAdjustment = PointerMath::alignForwardAdjustment(pstart, alignof(u16))
											+ PointerMath::ALIGN_ARR_HEADER<u16>::ret;
		const size_t	restMemory = gLinearSize - headAdjustment * sizeof(u16);
		size_t			maxLength = restMemory / sizeof(u16);
		u16 *			pArr = allocator::AllocateArray<u16>(linearAllocator, maxLength);

		// dose the allocation success?
		error += pArr == nullptr;

		// and we will not get any other byte from the allocator.
		error += nullptr != allocator::AllocateNew<u8>(linearAllocator);
		
		return error == 0;
	TEST_UNIT_END;

	TEST_UNIT_START("test malloc from gLinearAllocator")
		DEBUG_MESSAGE("TestStruct size: %d\n", sizeof(TestStruct));
		DEBUG_MESSAGE("TestStruct align: %d\n", alignof(TestStruct));
		allocator::LinearAllocator linearAllocator(gLinearSize, malloc(gLinearSize));
		Cleaner clearLinearAlc([&]() {linearAllocator.clear(); });
		debug::debugAllocator::LinearMemoryTracer tracer(linearAllocator);

		int error = 0;

		auto * shouldBeOne = allocator::AllocateNew<int>(linearAllocator);
		tracer.New<int>();
		*shouldBeOne = 1;

		error += tracer.report();

		const size_t arrLength = 5;
		auto * pTestStructArr = allocator::AllocateArray<TestStruct>(linearAllocator, arrLength);
		tracer.NewArray<TestStruct>(arrLength);
		for (int i = 0; i < arrLength; ++i)
		{
			error += NOT_EQ(1, pTestStructArr[i]._a);
			error += NOT_EQ(2, pTestStructArr[i]._b);
			error += NOT_EQ(3, pTestStructArr[i]._c);
		}

		const size_t longArrLength = 5;
		auto plongArr = allocator::AllocateArray<unsigned long>(linearAllocator, longArrLength);
		tracer.NewArray<unsigned long>(longArrLength);

		error += 1 != *shouldBeOne;
		error += tracer.report();

		// WARNING this code will cause tracer dismatch the linearAllocator,
		// because we want to test the tracer truly log the state
		// of the allocator.
		// so if corect, afther this operation,
		// the trace should return a number > 0.
		allocator::AllocateNew<int>(linearAllocator);

		// if it return 0,
		// there is some error in the tracer.
		error += tracer.report() == 0;

		//allocator::showAllocator(linearAllocator);
		return error == 0;
	TEST_UNIT_END;
}


int main()
{
	TestUnit::testMain();
    return 0;
}

