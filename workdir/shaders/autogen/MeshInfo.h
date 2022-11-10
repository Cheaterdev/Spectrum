#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/MeshInfo.h"
ConstantBuffer<MeshInfo> pass_MeshInfo: register( b2, space5);
const MeshInfo CreateMeshInfo()
{
	return pass_MeshInfo;
}
#ifndef NO_GLOBAL
static const MeshInfo meshInfo_global = CreateMeshInfo();
const MeshInfo GetMeshInfo(){ return meshInfo_global; }
#endif
