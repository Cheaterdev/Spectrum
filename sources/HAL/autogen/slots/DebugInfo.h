#pragma once
#include "..\Tables\DebugInfo.h"
namespace Slots {
	struct DebugInfo:public DataHolder<DebugInfo, SlotID::DebugInfo,Table::DebugInfo,FrameLayout::DebugInfo>
	{
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Slot;
		DebugInfo() = default;
	};
}
