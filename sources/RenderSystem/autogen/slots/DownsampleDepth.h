#pragma once
#include "..\Tables\DownsampleDepth.h"
namespace Slots {
	struct DownsampleDepth:public DataHolder<SlotID::DownsampleDepth,Table::DownsampleDepth,DefaultLayout::Instance0>
	{
		SRV srv;
		UAV uav;
		DownsampleDepth(): DataHolder(srv,uav){}
		DownsampleDepth(const DownsampleDepth&other): DataHolder(srv,uav){srv = other.srv;uav = other.uav;}
	};
}
