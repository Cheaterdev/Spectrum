#pragma once
#include "..\Tables\DownsampleDepth.h"
namespace Slots {
	struct DownsampleDepth:public DataHolder<DownsampleDepth, SlotID::DownsampleDepth,Table::DownsampleDepth,DefaultLayout::Instance0>
	{
		SRV srv;
		UAV uav;
		static inline const std::vector<UINT> tables = {DefaultLayout::Instance0::SRV_ID, DefaultLayout::Instance0::UAV_ID};
		DownsampleDepth(): DataHolder(srv,uav){}
		DownsampleDepth(const DownsampleDepth&other): DataHolder(srv,uav){srv = other.srv;uav = other.uav;}
	};
}
