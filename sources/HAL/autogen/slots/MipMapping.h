#pragma once
#include "..\Tables\MipMapping.h"
namespace Slots {
	struct MipMapping:public DataHolder<MipMapping, SlotID::MipMapping,Table::MipMapping,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		MipMapping() = default;
	};
}
