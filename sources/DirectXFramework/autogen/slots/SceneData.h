#pragma once
#include "..\Tables\SceneData.h"
namespace Slots {
	struct SceneData:public DataHolder<SceneData, SlotID::SceneData,Table::SceneData,FrameLayout::SceneData>
	{
		SceneData() = default;
	};
}
