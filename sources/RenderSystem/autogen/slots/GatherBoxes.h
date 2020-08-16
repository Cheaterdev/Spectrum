#pragma once
#include "..\Tables\GatherBoxes.h"
namespace Slots {
	struct GatherBoxes:public DataHolder<Table::GatherBoxes,DefaultLayout::Instance1>
	{
		UAV uav;
		GatherBoxes(): DataHolder(uav){}
		GatherBoxes(const GatherBoxes&other): DataHolder(uav){uav = other.uav;}
	};
}
