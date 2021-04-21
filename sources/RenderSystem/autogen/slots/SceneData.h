#pragma once
#include "..\Tables\SceneData.h"
namespace Slots {
	struct SceneData:public DataHolder<SlotID::SceneData,Table::SceneData,FrameLayout::SceneData>
	{
		SRV srv;
		Render::Bindless bindless;
		SceneData(): DataHolder(srv,bindless){}
		SceneData(const SceneData&other): DataHolder(srv,bindless){srv = other.srv;}
	};
}
