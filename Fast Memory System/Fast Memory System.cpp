#include "Mem.h"
#include <assert.h>
#include "Timer.h"
#include <stdio.h>


int main()
{
	Timer normalMem;
	Timer fastMem;

	//----------Normal Malloc/Free Speed Test--------

	normalMem.StartTimer();

	void* a0 = malloc(0x100);
	void* b0 = malloc(0x200);
	void* c0 = malloc(0x300);
	void* d0 = malloc(0x1300);
	void* e0 = malloc(0x300);
	void* f0 = malloc(0x900);
	void* g0 = malloc(0x600);

	void* a1 = malloc(0x800);
	void* b1 = malloc(0x200);
	void* c1 = malloc(0x1300);
	void* d1 = malloc(0x500);
	void* e1 = malloc(0x200);
	void* f1 = malloc(0x700);
	void* g1 = malloc(0x300);

	void* a2 = malloc(0x100);
	void* b2 = malloc(0x200);
	void* c2 = malloc(0x1300);
	void* d2 = malloc(0x300);
	void* e2 = malloc(0x800);
	void* f2 = malloc(0x100);
	void* g2 = malloc(0x300);

	void* a3 = malloc(0x2200);
	void* b3 = malloc(0x200);
	void* c3 = malloc(0x1300);
	void* d3 = malloc(0x800);
	void* e3 = malloc(0x300);
	void* f3 = malloc(0x100);
	void* g3 = malloc(0x300);

	free(a0);
	free(b0);
	free(c0);
	free(d0);
	free(e0);
	free(f0);
	free(g0);

	free(a1);
	free(b1);
	free(c1);
	free(d1);
	free(e1);
	free(f1);
	free(g1);

	free(a2);
	free(b2);
	free(c2);
	free(d2);
	free(e2);
	free(f2);
	free(g2);

	free(a3);
	free(b3);
	free(c3);
	free(d3);
	free(e3);
	free(f3);
	free(g3);

	normalMem.StopTimer();
	float time1 = normalMem.TimeInSeconds() * 1000.0f;

	//----------Fast Malloc/Free Speed Test----------
	Mem mem;
	mem.initialize();

	fastMem.StartTimer();

	void* a4 = mem.malloc(0x100);
	void* b4 = mem.malloc(0x200);
	void* c4 = mem.malloc(0x300);
	void* d4 = mem.malloc(0x1300);
	void* e4 = mem.malloc(0x300);
	void* f4 = mem.malloc(0x900);
	void* g4 = mem.malloc(0x600);
			   
	void* a5 = mem.malloc(0x800);
	void* b5 = mem.malloc(0x200);
	void* c5 = mem.malloc(0x1300);
	void* d5 = mem.malloc(0x500);
	void* e5 = mem.malloc(0x200);
	void* f5 = mem.malloc(0x700);
	void* g5 = mem.malloc(0x300);
			   
	void* a6 = mem.malloc(0x100);
	void* b6 = mem.malloc(0x200);
	void* c6 = mem.malloc(0x1300);
	void* d6 = mem.malloc(0x300);
	void* e6 = mem.malloc(0x800);
	void* f6 = mem.malloc(0x100);
	void* g6 = mem.malloc(0x300);
			   
	void* a7 = mem.malloc(0x2200);
	void* b7 = mem.malloc(0x200);
	void* c7 = mem.malloc(0x1300);
	void* d7 = mem.malloc(0x800);
	void* e7 = mem.malloc(0x300);
	void* f7 = mem.malloc(0x100);
	void* g7 = mem.malloc(0x300);

	mem.free(&a4);
	mem.free(&b4);
	mem.free(&c4);
	mem.free(&d4);
	mem.free(&e4);
	mem.free(&f4);
	mem.free(&g4);
	
	mem.free(&a5);
	mem.free(&b5);
	mem.free(&c5);
	mem.free(&d5);
	mem.free(&e5);
	mem.free(&f5);
	mem.free(&g5);
	
	mem.free(&a6);
	mem.free(&b6);
	mem.free(&c6);
	mem.free(&d6);
	mem.free(&e6);
	mem.free(&f6);
	mem.free(&g6);
			 
	mem.free(&a7);
	mem.free(&b7);
	mem.free(&c7);
	mem.free(&d7);
	mem.free(&e7);
	mem.free(&f7);
	mem.free(&g7);
	
	fastMem.StopTimer();
	float time2 = fastMem.TimeInSeconds() * 1000.0f;

	//---------Printing Results----------

	printf("Normal Malloc/Free:	%.4fms\n", time1);
	printf("Fast Malloc/Free:	%.4fms\n", time2);
	printf("%.2f times faster\n", time1 / time2);
}
