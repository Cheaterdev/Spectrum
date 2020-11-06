#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/MeshInfo.h"
ConstantBuffer<MeshInfo_cb> cb_4_0:register(b0,space4);
MeshInfo CreateMeshInfo()
{
	MeshInfo result;
	result.cb = cb_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const MeshInfo meshInfo_global = CreateMeshInfo();
const MeshInfo GetMeshInfo(){ return meshInfo_global; }
#endif
