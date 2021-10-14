#pragma once
#include "..\Tables\SceneData.h"
namespace Slots {
	struct SceneData:public DataHolder<SceneData, SlotID::SceneData,Table::SceneData,FrameLayout::SceneData>
	{
		SRV srv;
		Render::Bindless bindless;
		static inline const std::vector<UINT> tables = {FrameLayout::SceneData::SRV_ID};
		SceneData(): DataHolder(srv,bindless){}
		SceneData(const SceneData&other): DataHolder(srv,bindless){srv = other.srv;}
	};
}
