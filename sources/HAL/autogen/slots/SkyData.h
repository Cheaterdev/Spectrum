#pragma once
#include "..\Tables\SkyData.h"
namespace Slots {
	struct SkyData:public DataHolder<SkyData, SlotID::SkyData,Table::SkyData,DefaultLayout::Instance0>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		SkyData() = default;
	};
}
