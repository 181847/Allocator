// Allocator.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../../Library//MyTools/UnitTestModules.h"
#include "../Allocators/PointerMath.h"
#include "../Allocators/LinearAllocator.h"
#include "../Allocators/MemoryTracer.h"

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

	TEST_UNIT_START("test malloc from gLinearAllocator")
		DEBUG_MESSAGE("TestStruct size: %d\n", sizeof(TestStruct));
		DEBUG_MESSAGE("TestStruct align: %d\n", alignof(TestStruct));

		debug::debugAllocator::LinearMemoryTracer tracer(gLinearAllocator);
		

		int error = 0;
		auto * shouldBeOne = allocator::AllocateNew<int>(gLinearAllocator);
		tracer.New<int>();
		*shouldBeOne = 1;
		error += tracer.report();
		// after this allocating,
		// next pointer must align with 0x4.
		const size_t arrLength = 5;
		auto * pTestStructArr = allocator::AllocateArray<TestStruct>(gLinearAllocator, arrLength);
		tracer.NewArray<TestStruct>(arrLength);

		for (int i = 0; i < arrLength; ++i)
		{
			error += NOT_EQ(1, pTestStructArr[i]._a);
			error += NOT_EQ(2, pTestStructArr[i]._b);
			error += NOT_EQ(3, pTestStructArr[i]._c);
		}

		allocator::AllocateNew<int>(gLinearAllocator);

		error += 1 != *shouldBeOne;
		error += tracer.report();

		allocator::showAllocator(gLinearAllocator);
		return error == 0;
	TEST_UNIT_END;
}


int main()
{

	TestUnit::testMain();
    return 0;
}

