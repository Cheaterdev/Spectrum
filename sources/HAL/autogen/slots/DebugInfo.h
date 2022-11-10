#pragma once
#include "..\Tables\DebugInfo.h"
namespace Slots {
	struct DebugInfo:public DataHolder<DebugInfo, SlotID::DebugInfo,Table::DebugInfo,FrameLayout::DebugInfo>
	{
		DebugInfo() = default;
	};
}
