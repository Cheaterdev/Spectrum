#pragma once
#include "..\Tables\MaterialInfo.h"
namespace Slots {
	struct MaterialInfo:public DataHolder<Table::MaterialInfo,FrameLayout::MaterialData>
	{
		CB cb;
		Render::Bindless bindless;
		MaterialInfo(): DataHolder(cb,bindless){}
	};
}
