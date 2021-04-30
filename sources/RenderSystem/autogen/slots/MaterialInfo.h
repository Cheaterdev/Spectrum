#pragma once
#include "..\Tables\MaterialInfo.h"
namespace Slots {
	struct MaterialInfo:public DataHolder<MaterialInfo, SlotID::MaterialInfo,Table::MaterialInfo,DefaultLayout::MaterialData>
	{
		CB cb;
		static inline const std::vector<UINT> tables = {DefaultLayout::MaterialData::CB_ID};
		MaterialInfo(): DataHolder(cb){}
		MaterialInfo(const MaterialInfo&other): DataHolder(cb){cb = other.cb;}
	};
}
