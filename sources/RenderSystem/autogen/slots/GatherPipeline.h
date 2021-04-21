#pragma once
#include "..\Tables\GatherPipeline.h"
namespace Slots {
	struct GatherPipeline:public DataHolder<SlotID::GatherPipeline,Table::GatherPipeline,DefaultLayout::Instance1>
	{
		CB cb;
		UAV uav;
		GatherPipeline(): DataHolder(cb,uav){}
		GatherPipeline(const GatherPipeline&other): DataHolder(cb,uav){cb = other.cb;uav = other.uav;}
	};
}
