#pragma once
#include "..\Tables\SceneData.h"
namespace Slots {
	struct SceneData:public DataHolder<Table::SceneData,FrameLayout::SceneData>
	{
		SRV srv;
		DX12::Bindless bindless;
		SceneData(): DataHolder(srv,bindless){}
		SceneData(const SceneData&other): DataHolder(srv,bindless){srv = other.srv;}
	};
}
