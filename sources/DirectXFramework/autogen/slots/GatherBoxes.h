#pragma once
#include "..\Tables\GatherBoxes.h"
namespace Slots {
	struct GatherBoxes:public DataHolder<GatherBoxes, SlotID::GatherBoxes,Table::GatherBoxes,DefaultLayout::Instance1>
	{
		UAV uav;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance1::UAV_ID};
		GatherBoxes(): DataHolder(uav){}
		GatherBoxes(const GatherBoxes&other): DataHolder(uav){uav = other.uav;}
	};
}
