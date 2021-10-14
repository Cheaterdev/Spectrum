#pragma once
#include "..\Tables\DebugInfo.h"
namespace Slots {
	struct DebugInfo:public DataHolder<DebugInfo, SlotID::DebugInfo,Table::DebugInfo,FrameLayout::DebugInfo>
	{
		UAV uav;
		static inline const std::vector<UINT> tables = {FrameLayout::DebugInfo::UAV_ID};
		DebugInfo(): DataHolder(uav){}
		DebugInfo(const DebugInfo&other): DataHolder(uav){uav = other.uav;}
	};
}
