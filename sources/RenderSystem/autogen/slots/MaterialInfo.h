#pragma once
#include "..\Tables\MaterialInfo.h"
namespace Slots {
	struct MaterialInfo:public DataHolder<Table::MaterialInfo,FrameLayout::MaterialData>
	{
		CB cb;
		MaterialInfo(): DataHolder(cb){}
		MaterialInfo(const MaterialInfo&other): DataHolder(cb){cb = other.cb;}
	};
}
