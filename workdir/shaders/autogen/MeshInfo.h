#ifndef SLOT_5
	#define SLOT_5
#else
	#error Slot 5 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/MeshInfo.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_MeshInfo: register( b2, space5);
 ConstantBuffer<MeshInfo> CreateMeshInfo()
{
	return ResourceDescriptorHeap[pass_MeshInfo.offset];
}
#ifndef NO_GLOBAL
static const MeshInfo meshInfo_global = CreateMeshInfo();
const MeshInfo GetMeshInfo(){ return meshInfo_global; }
#endif
