// Allocator.cpp : 定义控制台应用程序的入口点。
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

	TEST_UNIT_START("forward alignment")
		void* addr1 = reinterpret_cast<void *>(0x00000001);
		void* addr2 = reinterpret_cast<void *>(0x1234567a);
		
		int success = 0;

		success += AddrEQ(0x00000002, ForwardAlign(addr1, 2));
		success += AddrEQ(0x00000004, ForwardAlign(addr1, 4));
		success += AddrEQ(0x00000008, ForwardAlign(addr1, 8));
		success += AddrEQ(0x00000010, ForwardAlign(addr1, 16));
		success += AddrEQ(0x00000020, ForwardAlign(addr1, 32));
		success += AddrEQ(0x1234567c, ForwardAlign(addr2, 4));
		success += AddrEQ(0x12345680, ForwardAlign(addr2, 8));
		success += AddrEQ(0x12345680, ForwardAlign(addr2, 16));
		success += AddrEQ(0x12345680, ForwardAlign(addr2, 32));
		return success == 9;
	TEST_UNIT_END;
}


int main()
{

	TestUnit::testMain();
    return 0;
}

