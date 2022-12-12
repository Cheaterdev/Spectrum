#ifndef SLOT_6
	#define SLOT_6
#else
	#error Slot 6 is already used
#endif
#include "layout/DefaultLayout.h"
#include "tables/MeshInstanceInfo.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_MeshInstanceInfo: register( b2, space6);
 ConstantBuffer<MeshInstanceInfo> CreateMeshInstanceInfo()
{
	return ResourceDescriptorHeap[pass_MeshInstanceInfo.offset];
}
#ifndef NO_GLOBAL
static const MeshInstanceInfo meshInstanceInfo_global = CreateMeshInstanceInfo();
const MeshInstanceInfo GetMeshInstanceInfo(){ return meshInstanceInfo_global; }
#endif
