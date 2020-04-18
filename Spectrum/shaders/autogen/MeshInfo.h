#ifndef SLOT_3
	#define SLOT_3
#else
	#error Slot 3 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/MeshInfo.h"
ConstantBuffer<MeshInfo_cb> cb_3_0:register(b0,space3);
MeshInfo CreateMeshInfo()
{
	MeshInfo result;
	result.cb = cb_3_0;
	return result;
}
static const MeshInfo meshInfo_global = CreateMeshInfo();
const MeshInfo GetMeshInfo(){ return meshInfo_global; }
