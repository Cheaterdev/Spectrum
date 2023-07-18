#pragma once
#include "..\Tables\SMAA_Blend.h"
namespace Slots {
	struct SMAA_Blend:public DataHolder<SMAA_Blend, SlotID::SMAA_Blend,Table::SMAA_Blend,DefaultLayout::Instance1>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		SMAA_Blend() = default;
	};
}
