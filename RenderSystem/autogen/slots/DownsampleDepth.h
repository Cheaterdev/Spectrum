#pragma once
#include "..\Tables\DownsampleDepth.h"
namespace Slots {
	struct DownsampleDepth:public DataHolder<Table::DownsampleDepth,DefaultLayout::Instance0>
	{
		SRV srv;
		UAV uav;
		DownsampleDepth(): DataHolder(srv,uav){}
	};
}
