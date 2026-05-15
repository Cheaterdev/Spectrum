#pragma once
		   extern "C++"{
struct BSPoolHandle;

BSPoolHandle* BSPool_Create();
void          BSPool_Destroy(BSPoolHandle* h);
void          BSPool_Submit(BSPoolHandle* h, void* ctx, void(*fn)(void*));
		   }