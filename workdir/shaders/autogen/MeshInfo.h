#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/MeshInfo.h"
ConstantBuffer<MeshInfo_cb> cb_5_0:register(b0,space5);
struct Pass_MeshInfo
{
};
ConstantBuffer<Pass_MeshInfo> pass_MeshInfo: register( b2, space5);
const MeshInfo CreateMeshInfo()
{
	MeshInfo result;
	Pass_MeshInfo pass;
	result.cb = cb_5_0;
	return result;
}
#ifndef NO_GLOBAL
static const MeshInfo meshInfo_global = CreateMeshInfo();
const MeshInfo GetMeshInfo(){ return meshInfo_global; }
#endif
