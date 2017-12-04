// TestAllocator.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../../Library/MyTools/MyTools.h"
#include "../Allocators/PointerMath.h"
#include "../Allocators/Allocator.h"

class A
{
public:
	int data = 5;

	A()
		:data(4)
	{
	}
	~A()
	{
		printf("A is be deleted, data is %d\n", data);
	}
	void Show()
	{
		printf("data is %d\n", data);
	}
};

int main()
{
	IF__<true, int, double>::reType myint;
	printf("%d", sizeof(void*));

	void* p1 = (void*)(0x1234567A);
	void* p2 = (void*)(0x00000011);

	PointerMath::printAddress(p1);
	printf("4\t");
	PointerMath::printAddress(PointerMath::alignForward(p1, 4));
	printf("8\t");
	PointerMath::printAddress(PointerMath::alignForward(p1, 8));
	printf("16\t");
	PointerMath::printAddress(PointerMath::alignForward(p1, 16));
	printf("32\t");
	PointerMath::printAddress(PointerMath::alignForward(p1, 32));

	printf("\n\n");
	PointerMath::printAddress(p2);
	printf("4\t");
	PointerMath::printAddress(PointerMath::alignForward(p2, 4));
	printf("adjust by %8x\n", PointerMath::alignForwardAdjustment(p2, 4));
	printf("8\t");
	PointerMath::printAddress(PointerMath::alignForward(p2, 8));
	printf("adjust by %8x\n", PointerMath::alignForwardAdjustment(p2, 8));
	printf("16\t");
	PointerMath::printAddress(PointerMath::alignForward(p2, 16));
	printf("adjust by %8x\n", PointerMath::alignForwardAdjustment(p2, 16));
	printf("32\t");
	PointerMath::printAddress(PointerMath::alignForward(p2, 32));
	printf("adjust by %8x\n", PointerMath::alignForwardAdjustment(p2, 32));

	void * pnew = malloc(sizeof(A));

	A* pnewa = reinterpret_cast<A*>(pnew);

	pnewa->Show();

	new (pnewa) A;
	printf("after\n");
	pnewa->Show();

	(*pnewa).~A();

	printf("sizeof (size_t): %d\n", sizeof(size_t));

	int align  = allocator::ALIGN_ARR_HEADER<unsigned char>::ret;

	printf("align %d \n", align);

	getchar();
	return 0;
}

