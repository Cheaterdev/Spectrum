// Non-module TU: no module declaration, no import std — isolated from the module system entirely.
#include "BS_thread_pool.hpp"
#include "BSPoolBridge.h"

struct BSPoolHandle
{
	BS::thread_pool pool;
};

BSPoolHandle* BSPool_Create()
{
	return new BSPoolHandle();
}

void BSPool_Destroy(BSPoolHandle* h)
{
	delete h;
}

void BSPool_Submit(BSPoolHandle* h, void* ctx, void(*fn)(void*))
{
	h->pool.submit_task([ctx, fn]() { fn(ctx); });
}
