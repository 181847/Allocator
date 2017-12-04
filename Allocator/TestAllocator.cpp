// Allocator.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "../../Library//MyTools/UnitTestModules.h"
#include "../Allocators/PointerMath.h"

DECLARE_TEST_UNITS;

#define AddrEQ(u32, addr2) (reinterpret_cast<void*>(u32) == addr2)

void TestUnit::GetReady()
{

}

void TestUnit::AfterTest()
{

}

void TestUnit::AddTestUnit()
{
	TEST_UNIT_START("first unit startup")
		return true;
	TEST_UNIT_END;

	TEST_UNIT_START("test alignForward")
		void* addr1 = reinterpret_cast<void *>(0x00000001);
		void* addr2 = reinterpret_cast<void *>(0x1234567a);
		
		int success = 0;

		success += AddrEQ(0x00000002, PointerMath::alignForward(addr1, 2));
		success += AddrEQ(0x00000004, PointerMath::alignForward(addr1, 4));
		success += AddrEQ(0x00000008, PointerMath::alignForward(addr1, 8));
		success += AddrEQ(0x00000010, PointerMath::alignForward(addr1, 16));
		success += AddrEQ(0x00000020, PointerMath::alignForward(addr1, 32));
		success += AddrEQ(0x1234567c, PointerMath::alignForward(addr2, 4));
		success += AddrEQ(0x12345680, PointerMath::alignForward(addr2, 8));
		success += AddrEQ(0x12345680, PointerMath::alignForward(addr2, 16));
		success += AddrEQ(0x12345680, PointerMath::alignForward(addr2, 32));
		return success == 9;
	TEST_UNIT_END;

	TEST_UNIT_START("test alignForwardAdjustment")

		return false;
	TEST_UNIT_END;
}


int main()
{

	TestUnit::testMain();
    return 0;
}
