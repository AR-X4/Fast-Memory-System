#include "Mem.h"
#include "assert.h"


int main()
{
	Mem mem;
	mem.initialize();

	void* a0 = mem.malloc(0x100);
	assert(a0 != nullptr);

	void* b0 = mem.malloc(0x200);
	assert(b0 != nullptr);

	void* c0 = mem.malloc(0x300);
	assert(c0 != nullptr);

	void* d0 = mem.malloc(0x1300);
	assert(d0 != nullptr);

	void* e0 = mem.malloc(0x300);
	assert(e0 != nullptr);

	void* f0 = mem.malloc(0x900);
	assert(f0 != nullptr);

	void* g0 = mem.malloc(0x600);
	assert(g0 != nullptr);

	mem.free(a0);
	a0 = nullptr;
	mem.free(b0);
	b0 = nullptr;
	mem.free(c0);
	c0 = nullptr;
	mem.free(d0);
	d0 = nullptr;
	mem.free(e0);
	e0 = nullptr;
	mem.free(f0);
	f0 = nullptr;
	mem.free(g0);
	g0 = nullptr;

}

