#ifndef SLOT_1
	#define SLOT_1
#else
	#error Slot 1 is already used
#endif
#include "layout/FrameLayout.h"
#include "tables/SceneData.h"
#ifndef CB_DEFINED
#define CB_DEFINED
struct CB { uint offset; };
#endif
ConstantBuffer< CB > pass_SceneData: register( b2, space1);
 ConstantBuffer<SceneData> CreateSceneData()
{
	return ResourceDescriptorHeap[pass_SceneData.offset];
}
#ifndef NO_GLOBAL
static const SceneData sceneData_global = CreateSceneData();
const SceneData GetSceneData(){ return sceneData_global; }
#endif
