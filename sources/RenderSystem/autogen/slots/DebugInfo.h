#pragma once
#include "..\Tables\DebugInfo.h"
namespace Slots {
	struct DebugInfo:public DataHolder<Table::DebugInfo,FrameLayout::DebugInfo>
	{
		UAV uav;
		DebugInfo(): DataHolder(uav){}
		DebugInfo(const DebugInfo&other): DataHolder(uav){uav = other.uav;}
	};
}
