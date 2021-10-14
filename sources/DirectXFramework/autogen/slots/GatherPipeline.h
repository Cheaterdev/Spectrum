#pragma once
#include "..\Tables\GatherPipeline.h"
namespace Slots {
	struct GatherPipeline:public DataHolder<GatherPipeline, SlotID::GatherPipeline,Table::GatherPipeline,DefaultLayout::Instance1>
	{
		CB cb;
		UAV uav;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance1::CB_ID, DefaultLayout::Instance1::UAV_ID};
		GatherPipeline(): DataHolder(cb,uav){}
		GatherPipeline(const GatherPipeline&other): DataHolder(cb,uav){cb = other.cb;uav = other.uav;}
	};
}
