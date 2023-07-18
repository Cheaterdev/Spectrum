#pragma once
#include "..\Tables\MaterialInfo.h"
namespace Slots {
	struct MaterialInfo:public DataHolder<MaterialInfo, SlotID::MaterialInfo,Table::MaterialInfo,DefaultLayout::MaterialData>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		MaterialInfo() = default;
	};
}
