#pragma once
#include "..\Tables\SceneData.h"
namespace Slots {
	struct SceneData:public DataHolder<SceneData, SlotID::SceneData,Table::SceneData,FrameLayout::SceneData>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		SceneData() = default;
	};
}
