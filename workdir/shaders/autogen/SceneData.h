#ifndef SLOT_1
	#define SLOT_1
#else
	#error Slot 1 is already used
#endif
#include "layout/FrameLayout.h"
#include "tables/SceneData.h"
ConstantBuffer<SceneData> pass_SceneData: register( b2, space1);
const SceneData CreateSceneData()
{
	return pass_SceneData;
}
#ifndef NO_GLOBAL
static const SceneData sceneData_global = CreateSceneData();
const SceneData GetSceneData(){ return sceneData_global; }
#endif
