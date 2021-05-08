#ifndef SLOT_4
	#define SLOT_4
#else
	#error Slot 4 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/MeshInfo.h"
ConstantBuffer<MeshInfo_cb> cb_4_0:register(b0,space4);
struct Pass_MeshInfo
{
};
ConstantBuffer<Pass_MeshInfo> pass_MeshInfo: register( b2, space4);
const MeshInfo CreateMeshInfo()
{
	MeshInfo result;
	Pass_MeshInfo pass;
	result.cb = cb_4_0;
	return result;
}
#ifndef NO_GLOBAL
static const MeshInfo meshInfo_global = CreateMeshInfo();
const MeshInfo GetMeshInfo(){ return meshInfo_global; }
#endif
