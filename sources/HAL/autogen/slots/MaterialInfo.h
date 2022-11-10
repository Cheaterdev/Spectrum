#pragma once
#include "..\Tables\MaterialInfo.h"
namespace Slots {
	struct MaterialInfo:public DataHolder<MaterialInfo, SlotID::MaterialInfo,Table::MaterialInfo,DefaultLayout::MaterialData>
	{
		MaterialInfo() = default;
	};
}
